//
// Created by copilot on 3/16/2026.
//

#include "mla_stream.h"
#include "../log/mla_logging.h"

///////////////////////////////////////////////////////////////////
/// DEFLATE Internal Constants and Structures (RFC 1951)
///////////////////////////////////////////////////////////////////

// DEFLATE block types
#define mla_deflate_block_uncompressed  0
#define mla_deflate_block_fixed         1
#define mla_deflate_block_dynamic       2

// DEFLATE limits
#define mla_deflate_max_bits            15
#define mla_deflate_max_lit_codes       286
#define mla_deflate_fixed_lit_codes     288       // RFC 1951 §3.2.6: fixed Huffman uses 288 code-space entries (0..287)
#define mla_deflate_max_dist_codes      30
#define mla_deflate_max_cl_codes        19
#define mla_deflate_max_codes           (mla_deflate_max_lit_codes + mla_deflate_max_dist_codes)
#define mla_deflate_end_of_block        256

///////////////////////////////////////////////////////////////////
/// Memory Usage Profiles
///
/// Control RAM vs compression-ratio trade-off by defining
/// mla_deflate_memory_usage before including this file or by
/// passing -Dmla_deflate_memory_usage=<profile> to the compiler.
///
///   mla_deflate_memory_small  (default)
///       Lowest possible footprint – ideal for ESP32 and other
///       MCUs with limited SRAM.
///       Compress context : ~9 KB    Decompress context : ~3 KB
///
///   mla_deflate_memory_medium
///       Balanced trade-off between memory and compression ratio –
///       suitable for desktop / server systems.
///       Compress context : ~36 KB   Decompress context : ~13 KB
///
///   mla_deflate_memory_high
///       Best compression ratio – maximum window and hash table,
///       deep chain search. Use on server / high-end systems where
///       RAM is plentiful and output size matters most.
///       Compress context : ~130 KB  Decompress context : ~50 KB
///////////////////////////////////////////////////////////////////
#define mla_deflate_memory_small   1
#define mla_deflate_memory_medium  2
#define mla_deflate_memory_high    3

#if !defined(mla_deflate_memory_usage)
    #define mla_deflate_memory_usage mla_deflate_memory_small
#endif

#if mla_deflate_memory_usage == mla_deflate_memory_small
    // --- small: ESP32 / low-RAM embedded targets ---
    #define mla_deflate_window_size       2048   // LZ77 sliding window (bytes)
    #define mla_deflate_hash_bits         10     // hash table =  1 024 entries
    #define mla_deflate_compress_buf_size 512    // output write buffer (bytes)
    #define mla_deflate_chain_limit       16     // max hash-chain steps per byte
#elif mla_deflate_memory_usage == mla_deflate_memory_medium
    // --- medium: desktop / server ---
    #define mla_deflate_window_size       8192   // LZ77 sliding window (bytes)
    #define mla_deflate_hash_bits         12     // hash table =  4 096 entries
    #define mla_deflate_compress_buf_size 4096   // output write buffer (bytes)
    #define mla_deflate_chain_limit       64     // max hash-chain steps per byte
#elif mla_deflate_memory_usage == mla_deflate_memory_high
    // --- high: best compression, server / high-end systems ---
    #define mla_deflate_window_size       32768  // LZ77 sliding window (bytes) – DEFLATE max
    #define mla_deflate_hash_bits         14     // hash table = 16 384 entries
    #define mla_deflate_compress_buf_size 16384  // output write buffer (bytes)
    #define mla_deflate_chain_limit       256    // max hash-chain steps per byte
#else
    #error "Unknown mla_deflate_memory_usage value. Use mla_deflate_memory_small, mla_deflate_memory_medium or mla_deflate_memory_high."
#endif

// Derived constants – do not edit these directly
#define mla_deflate_window_mask         (mla_deflate_window_size - 1)
#define mla_deflate_min_match           3
#define mla_deflate_max_match           258
#define mla_deflate_hash_size           (1 << mla_deflate_hash_bits)
#define mla_deflate_hash_mask           (mla_deflate_hash_size - 1)


// Code length alphabet order (RFC 1951 section 3.2.7)
static const mla_uint8_t __mla_deflate_cl_order[mla_deflate_max_cl_codes] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

// Length code base values (codes 257..285)
static const mla_uint16_t __mla_deflate_length_base[29] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
    15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
    67, 83, 99, 115, 131, 163, 195, 227, 258
};

// Length code extra bits (codes 257..285)
static const mla_uint8_t __mla_deflate_length_extra[29] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
    4, 4, 4, 4, 5, 5, 5, 5, 0
};

// Distance code base values (codes 0..29)
static const mla_uint16_t __mla_deflate_dist_base[30] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25,
    33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
    1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};

// Distance code extra bits (codes 0..29)
static const mla_uint8_t __mla_deflate_dist_extra[30] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
    4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
    9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

///////////////////////////////////////////////////////////////////
/// Huffman Decode Table
///////////////////////////////////////////////////////////////////

template<mla_uint16_t TSymbolCapacity>
struct __mla_deflate_huffman_t {
    mla_uint16_t counts[mla_deflate_max_bits + 1]; // Number of symbols per bit length
    mla_uint16_t symbols[TSymbolCapacity];   // Sorted symbols
    mla_uint32_t first_code[mla_deflate_max_bits + 1];  // Pre-computed first canonical code per bit length
    mla_uint16_t first_index[mla_deflate_max_bits + 1];  // Pre-computed symbol index offset per bit length
    mla_uint16_t num_symbols;
};

template<mla_uint16_t TSymbolCapacity>
__mla_deflate_huffman_t<TSymbolCapacity> __mla_deflate_huffman_empty() {
    return { {}, {}, {}, {}, 0 };
}

template<mla_uint16_t TSymbolCapacity>
static void __mla_deflate_huffman_init(__mla_deflate_huffman_t<TSymbolCapacity> &table) {
    mla_memset(&table, 0, sizeof(__mla_deflate_huffman_t<TSymbolCapacity>));
}

// Build a Huffman decode table from code lengths
template<mla_uint16_t TSymbolCapacity>
static mla_bool_t __mla_deflate_huffman_build(
    __mla_deflate_huffman_t<TSymbolCapacity> &table,
    const mla_uint8_t *lengths,
    mla_uint16_t num_symbols)
{
    mla_memset(table.counts,     0, sizeof(table.counts));
    mla_memset(table.symbols,    0, sizeof(table.symbols));
    mla_memset(table.first_code, 0, sizeof(table.first_code));
    mla_memset(table.first_index,0, sizeof(table.first_index));
    table.num_symbols = num_symbols;

    // Count the number of codes for each bit length
    for (mla_uint16_t i = 0; i < num_symbols; i++) {
        if (lengths[i] > mla_deflate_max_bits)
            return false;
        table.counts[lengths[i]]++;
    }
    table.counts[0] = 0;

    // ── Compute offset table for sorting symbols ─────────────────────────
    mla_uint16_t offsets[mla_deflate_max_bits + 1];
    offsets[0] = 0;
    offsets[1] = 0;
    for (mla_uint16_t i = 1; i < mla_deflate_max_bits; i++) {
        offsets[i + 1] = offsets[i] + table.counts[i];
    }

    // ── Sort symbols by code length, then by symbol value ────────────────
    for (mla_uint16_t i = 0; i < num_symbols; i++) {
        if (lengths[i] != 0) {
            mla_uint16_t slot = offsets[lengths[i]];
            if (slot < TSymbolCapacity) {          // compile-time guard
                table.symbols[slot] = i;
            }
            offsets[lengths[i]]++;
        }
    }
    // ─────────────────────────────────────────────────────────────────────

    // Pre-compute first canonical code and symbol index for each bit length
    // This avoids per-bit reversal during decoding
    mla_uint32_t code = 0;
    mla_uint16_t index = 0;
    for (mla_uint8_t len = 1; len <= mla_deflate_max_bits; len++) {
        table.first_code[len]  = code;
        table.first_index[len] = index;
        index += table.counts[len];
        code   = (code + table.counts[len]) << 1;
    }

    return true;
}

///////////////////////////////////////////////////////////////////
/// Bit Reader (for decompression)
///////////////////////////////////////////////////////////////////

struct __mla_deflate_bit_reader_t {
    mla_uint32_t bit_buffer;
    mla_uint8_t bit_count;
    mla_byte_t prefetched_bytes[2];
    mla_uint8_t prefetched_length;
    mla_uint8_t prefetched_position;
    mla_bool_t error;
};

static void __mla_deflate_bit_reader_init(__mla_deflate_bit_reader_t &reader) {
    reader.bit_buffer = 0;
    reader.bit_count = 0;
    reader.prefetched_length = 0;
    reader.prefetched_position = 0;
    reader.error = false;
}

static mla_bool_t __mla_deflate_bit_reader_read_byte(__mla_deflate_bit_reader_t &reader, mla_stream_input_t &input, mla_byte_t &byte_val) {
    if (reader.prefetched_position < reader.prefetched_length) {
        byte_val = reader.prefetched_bytes[reader.prefetched_position++];
        return true;
    }

    mla_size_t read_count = input.read(input, 0, 1, &byte_val);
    if (read_count == 0) {
        reader.error = true;
        return false;
    }

    return true;
}

static mla_bool_t __mla_deflate_bit_reader_ensure(__mla_deflate_bit_reader_t &reader, mla_stream_input_t &input, mla_uint8_t bits_needed) {
    while (reader.bit_count < bits_needed) {
        mla_byte_t byte_val = 0;
        if (!__mla_deflate_bit_reader_read_byte(reader, input, byte_val)) {
            return false;
        }
        reader.bit_buffer |= ((mla_uint32_t)byte_val) << reader.bit_count;
        reader.bit_count += 8;
    }
    return true;
}

static mla_uint32_t __mla_deflate_bit_reader_read(__mla_deflate_bit_reader_t &reader, mla_stream_input_t &input, mla_uint8_t num_bits) {
    if (num_bits == 0) return 0;

    if (!__mla_deflate_bit_reader_ensure(reader, input, num_bits)) {
        return 0;
    }

    mla_uint32_t value = reader.bit_buffer & ((1u << num_bits) - 1);
    reader.bit_buffer >>= num_bits;
    reader.bit_count -= num_bits;
    return value;
}

// Decode a Huffman code from the bit stream using pre-computed first codes
// Reads bits one at a time and builds the canonical code directly without bit reversal
template<mla_uint16_t TSymbolCapacity>
static mla_int32_t __mla_deflate_huffman_decode(__mla_deflate_bit_reader_t &reader, mla_stream_input_t &input, const __mla_deflate_huffman_t<TSymbolCapacity> &table)
{
    mla_uint32_t code = 0;

    for (mla_uint8_t len = 1; len <= mla_deflate_max_bits; len++) {
        if (!__mla_deflate_bit_reader_ensure(reader, input, len)) {
            return -1;
        }

        // Read next bit from LSB of bit_buffer and build canonical (MSB-first) code directly
        mla_uint32_t bit = (reader.bit_buffer >> (len - 1)) & 1;
        code = (code << 1) | bit;

        // Use pre-computed first_code and first_index for direct lookup
        mla_uint32_t count = table.counts[len];
        if (count > 0 && code >= table.first_code[len] && code - table.first_code[len] < count) {
            mla_uint16_t sym_index = table.first_index[len] + (mla_uint16_t)(code - table.first_code[len]);
            if (sym_index >= TSymbolCapacity) {   // bounds guard
                reader.error = true;
                return -1;
            }
            // Consume the bits
            reader.bit_buffer >>= len;
            reader.bit_count -= len;
            return (mla_int32_t)table.symbols[sym_index];
        }
    }

    reader.error = true;
    return -1;
}

// Discard bits to align to a byte boundary
static void __mla_deflate_bit_reader_align(__mla_deflate_bit_reader_t &reader) {
    mla_uint8_t discard = reader.bit_count & 7;
    reader.bit_buffer >>= discard;
    reader.bit_count -= discard;
}

static mla_size_t __mla_deflate_window_bits_value() {
    mla_size_t windowSize = (mla_size_t)mla_deflate_window_size;
    mla_size_t bits = 0;
    while (windowSize > 1) {
        windowSize >>= 1;
        bits++;
    }
    return bits;
}

static mla_uint32_t __mla_deflate_adler32_update(mla_uint32_t current, const mla_byte_t *data, mla_size_t length) {
    const mla_uint32_t mod_adler = 65521u;
    mla_uint32_t sum1 = current & 0xFFFFu;
    mla_uint32_t sum2 = (current >> 16) & 0xFFFFu;

    for (mla_size_t i = 0; i < length; i++) {
        sum1 += (mla_uint32_t)data[i];
        if (sum1 >= mod_adler) {
            sum1 -= mod_adler;
        }

        sum2 += sum1;
        sum2 %= mod_adler;
    }

    return (sum2 << 16) | sum1;
}

static mla_uint32_t __mla_deflate_adler32_update_byte(mla_uint32_t current, mla_byte_t value) {
    return __mla_deflate_adler32_update(current, &value, 1);
}

///////////////////////////////////////////////////////////////////
/// CRC-32 (ISO 3309 / ITU-T V.42) – required for gzip (RFC 1952)
///////////////////////////////////////////////////////////////////

static mla_uint32_t __mla_deflate_crc32_table[256];
static mla_bool_t   __mla_deflate_crc32_table_initialized = false;

static void __mla_deflate_crc32_build_table() {
    if (__mla_deflate_crc32_table_initialized) return;
    for (mla_uint32_t i = 0; i < 256; i++) {
        mla_uint32_t c = i;
        for (mla_uint8_t k = 0; k < 8; k++) {
            if (c & 1u) {
                c = 0xEDB88320u ^ (c >> 1);
            } else {
                c >>= 1;
            }
        }
        __mla_deflate_crc32_table[i] = c;
    }
    __mla_deflate_crc32_table_initialized = true;
}

static mla_uint32_t __mla_deflate_crc32_update(mla_uint32_t crc, const mla_byte_t *data, mla_size_t length) {
    __mla_deflate_crc32_build_table();
    crc = ~crc;
    for (mla_size_t i = 0; i < length; i++) {
        crc = __mla_deflate_crc32_table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
    }
    return ~crc;
}

static mla_uint32_t __mla_deflate_crc32_update_byte(mla_uint32_t crc, mla_byte_t value) {
    return __mla_deflate_crc32_update(crc, &value, 1);
}

static mla_bool_t __mla_deflate_is_zlib_header(mla_byte_t cmf, mla_byte_t flg) {
    if ((cmf & 0x0F) != 8) {
        return false;
    }

    if ((cmf >> 4) > 7) {
        return false;
    }

    mla_uint16_t header = (mla_uint16_t)(((mla_uint16_t)cmf << 8) | flg);
    return (header % 31u) == 0;
}

///////////////////////////////////////////////////////////////////
/// Bit Writer (for compression)
///////////////////////////////////////////////////////////////////

struct __mla_deflate_bit_writer_t {
    mla_byte_t buffer[mla_deflate_compress_buf_size];
    mla_size_t buffer_pos;
    mla_uint32_t bit_buffer;
    mla_uint8_t bit_count;
    mla_bool_t error;
};

static void __mla_deflate_bit_writer_init(__mla_deflate_bit_writer_t &writer) {
    writer.buffer_pos = 0;
    writer.bit_buffer = 0;
    writer.bit_count = 0;
    writer.error = false;
}

static void __mla_deflate_bit_writer_flush_buffer(__mla_deflate_bit_writer_t &writer, mla_stream_output_t &output) {
    if (writer.buffer_pos > 0) {
        mla_size_t written = output.write(output, 0, writer.buffer_pos, writer.buffer);
        if (written != writer.buffer_pos) {
            writer.error = true;
        }
        writer.buffer_pos = 0;
    }
}

static void __mla_deflate_bit_writer_put_byte(__mla_deflate_bit_writer_t &writer, mla_stream_output_t &output, mla_byte_t byte_val) {
    writer.buffer[writer.buffer_pos++] = byte_val;
    if (writer.buffer_pos >= mla_deflate_compress_buf_size) {
        __mla_deflate_bit_writer_flush_buffer(writer, output);
    }
}

static void __mla_deflate_bit_writer_write(__mla_deflate_bit_writer_t &writer, mla_stream_output_t &output, mla_uint32_t value, mla_uint8_t num_bits) {
    writer.bit_buffer |= value << writer.bit_count;
    writer.bit_count += num_bits;

    while (writer.bit_count >= 8) {
        __mla_deflate_bit_writer_put_byte(writer, output, (mla_byte_t)(writer.bit_buffer & 0xFF));
        writer.bit_buffer >>= 8;
        writer.bit_count -= 8;
    }
}

// Write bits in reversed order (MSB first, for Huffman codes)
static void __mla_deflate_bit_writer_write_reversed(__mla_deflate_bit_writer_t &writer, mla_stream_output_t &output, mla_uint32_t code, mla_uint8_t length) {
    mla_uint32_t reversed = 0;
    for (mla_uint8_t i = 0; i < length; i++) {
        reversed = (reversed << 1) | (code & 1);
        code >>= 1;
    }
    __mla_deflate_bit_writer_write(writer, output, reversed, length);
}

static void __mla_deflate_bit_writer_align(__mla_deflate_bit_writer_t &writer, mla_stream_output_t &output) {
    if (writer.bit_count > 0) {
        __mla_deflate_bit_writer_put_byte(writer, output, (mla_byte_t)(writer.bit_buffer & 0xFF));
        writer.bit_buffer = 0;
        writer.bit_count = 0;
    }
}

///////////////////////////////////////////////////////////////////
/// Fixed Huffman Tables (RFC 1951 section 3.2.6)
///////////////////////////////////////////////////////////////////

struct __mla_deflate_fixed_tables_t {
    mla_uint16_t lit_code[mla_deflate_fixed_lit_codes];
    mla_uint8_t lit_length[mla_deflate_fixed_lit_codes];
    mla_bool_t initialized;
};

static __mla_deflate_fixed_tables_t __mla_deflate_fixed_tables = { {}, {}, false };

static void __mla_deflate_build_fixed_tables() {

    if (__mla_deflate_fixed_tables.initialized)
        return;

    // Build literal/length code lengths per RFC 1951 3.2.6:
    //   0..143   -> 8 bits
    //   144..255 -> 9 bits
    //   256..279 -> 7 bits
    //   280..287 -> 8 bits
    // Note: symbols 286-287 are unused in practice, but RFC 1951 defines
    // 288 entries for the fixed Huffman code space and all 288 must be
    // included so that the canonical code assignment is correct.
    mla_uint16_t i;
    for (i = 0; i <= 143; i++) __mla_deflate_fixed_tables.lit_length[i] = 8;
    for (i = 144; i <= 255; i++) __mla_deflate_fixed_tables.lit_length[i] = 9;
    for (i = 256; i <= 279; i++) __mla_deflate_fixed_tables.lit_length[i] = 7;
    for (i = 280; i <= 287; i++) __mla_deflate_fixed_tables.lit_length[i] = 8;

    // Build the actual Huffman codes from the lengths
    // Step 1: Count the number of codes for each bit length
    mla_uint16_t bl_count[mla_deflate_max_bits + 1];
    mla_memset(bl_count, 0, sizeof(bl_count));
    for (i = 0; i < mla_deflate_fixed_lit_codes; i++) {
        bl_count[__mla_deflate_fixed_tables.lit_length[i]]++;
    }

    // Step 2: Find the numerical value of the smallest code for each code length
    mla_uint16_t next_code[mla_deflate_max_bits + 1];
    mla_memset(next_code, 0, sizeof(next_code));
    mla_uint32_t code = 0;
    for (mla_uint8_t bits = 1; bits <= mla_deflate_max_bits; bits++) {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = (mla_uint16_t)code;
    }

    // Step 3: Assign codes to symbols
    for (i = 0; i < mla_deflate_fixed_lit_codes; i++) {
        mla_uint8_t len = __mla_deflate_fixed_tables.lit_length[i];
        if (len != 0) {
            __mla_deflate_fixed_tables.lit_code[i] = next_code[len]++;
        }
    }

    __mla_deflate_fixed_tables.initialized = true;
}

///////////////////////////////////////////////////////////////////
/// Compression: LZ77 + Fixed Huffman Encoding
///////////////////////////////////////////////////////////////////

struct __mla_deflate_compress_state_t {
    mla_stream_output_t base_output;
    __mla_deflate_bit_writer_t writer;

    // Sliding window
    mla_byte_t window[mla_deflate_window_size];
    mla_size_t window_pos;
    mla_size_t window_filled;

    // Hash chain for LZ77 matching
    mla_uint16_t hash_head[mla_deflate_hash_size];    // Head of hash chain
    mla_uint16_t hash_prev[mla_deflate_window_size];  // Previous entry in hash chain

    mla_uint32_t adler32;
    mla_uint32_t crc32;
    mla_uint32_t input_size;
    mla_bool_t container_header_written;
    mla_bool_t block_started;
    mla_bool_t finished;
    mla_deflate_mode_t mode;
};

struct __mla_deflate_compress_state_initializer {

    static __mla_deflate_compress_state_t init() {
        __mla_deflate_compress_state_t result = {
            mla_stream_noop_output(),      // base_output
            { {}, 0, 0, 0, false}, // writer
            {}, 0, 0,                      // window, window_pos, window_filled
            {}, {},                        // hash_head, hash_prev
            1u, 0u, 0u,                   // adler32, crc32, input_size
            false,                         // container_header_written
            false, false,                  // block_started, finished
            mla_deflate_mode_raw // mode
        };
        return result;
    }
};

static void __mla_deflate_write_zlib_header(__mla_deflate_bit_writer_t &writer, mla_stream_output_t &output) {
    mla_uint8_t cinfo = (mla_uint8_t)(__mla_deflate_window_bits_value() - 8);
    mla_uint8_t cmf = (mla_uint8_t)((cinfo << 4) | 8u);
    mla_uint8_t flg = 0;
    mla_uint16_t header = (mla_uint16_t)(((mla_uint16_t)cmf << 8) | flg);
    flg = (mla_uint8_t)((31u - (header % 31u)) % 31u);

    __mla_deflate_bit_writer_put_byte(writer, output, cmf);
    __mla_deflate_bit_writer_put_byte(writer, output, flg);
}

static void __mla_deflate_write_gzip_header(__mla_deflate_bit_writer_t &writer, mla_stream_output_t &output) {
    __mla_deflate_bit_writer_put_byte(writer, output, 0x1F); // ID1
    __mla_deflate_bit_writer_put_byte(writer, output, 0x8B); // ID2
    __mla_deflate_bit_writer_put_byte(writer, output, 0x08); // CM = deflate
    __mla_deflate_bit_writer_put_byte(writer, output, 0x00); // FLG = no extras
    __mla_deflate_bit_writer_put_byte(writer, output, 0x00); // MTIME byte 0
    __mla_deflate_bit_writer_put_byte(writer, output, 0x00); // MTIME byte 1
    __mla_deflate_bit_writer_put_byte(writer, output, 0x00); // MTIME byte 2
    __mla_deflate_bit_writer_put_byte(writer, output, 0x00); // MTIME byte 3
    __mla_deflate_bit_writer_put_byte(writer, output, 0x00); // XFL
    __mla_deflate_bit_writer_put_byte(writer, output, 0xFF); // OS = unknown
}

static mla_bool_t __mla_deflate_compress_ensure_container_header(__mla_deflate_compress_state_t &state) {
    if (state.container_header_written) {
        return true;
    }

    if (state.mode == mla_deflate_mode_zlib) {
        __mla_deflate_write_zlib_header(state.writer, state.base_output);
    } else if (state.mode == mla_deflate_mode_gzip) {
        __mla_deflate_write_gzip_header(state.writer, state.base_output);
    }

    state.container_header_written = true;
    return !state.writer.error;
}

static mla_uint16_t __mla_deflate_hash3(const mla_byte_t *data) {
    mla_uint32_t h = ((mla_uint32_t)data[0] * 257) ^ ((mla_uint32_t)data[1] * 13) ^ (mla_uint32_t)data[2];
    return (mla_uint16_t)(h & mla_deflate_hash_mask);
}

// Find the length code index for a given length value
static mla_uint16_t __mla_deflate_length_code(mla_uint16_t length) {
    for (mla_uint16_t i = 0; i < 29; i++) {
        mla_uint16_t next_base = (i < 28) ? __mla_deflate_length_base[i + 1] : 259;
        if (length < next_base) {
            return i;
        }
    }
    return 28;
}

// Find the distance code index for a given distance value
static mla_uint16_t __mla_deflate_dist_code(mla_uint16_t distance) {
    for (mla_uint16_t i = 0; i < 30; i++) {
        mla_uint16_t next_base = (i < 29) ? __mla_deflate_dist_base[i + 1] : 32769;
        if (distance < next_base) {
            return i;
        }
    }
    return 29;
}

// Emit a literal byte using fixed Huffman coding
static void __mla_deflate_emit_literal(__mla_deflate_bit_writer_t &writer, mla_stream_output_t &output, mla_byte_t literal) {
    mla_uint16_t code = __mla_deflate_fixed_tables.lit_code[literal];
    mla_uint8_t length = __mla_deflate_fixed_tables.lit_length[literal];
    __mla_deflate_bit_writer_write_reversed(writer, output, code, length);
}

// Emit a length/distance pair using fixed Huffman coding
static void __mla_deflate_emit_match(__mla_deflate_bit_writer_t &writer, mla_stream_output_t &output, mla_uint16_t match_len, mla_uint16_t match_dist) {
    // Emit length code
    mla_uint16_t len_idx = __mla_deflate_length_code(match_len);
    mla_uint16_t len_symbol = 257 + len_idx;
    mla_uint16_t code = __mla_deflate_fixed_tables.lit_code[len_symbol];
    mla_uint8_t code_len = __mla_deflate_fixed_tables.lit_length[len_symbol];
    __mla_deflate_bit_writer_write_reversed(writer, output, code, code_len);

    // Emit length extra bits (LSB first, no reversal)
    mla_uint8_t extra = __mla_deflate_length_extra[len_idx];
    if (extra > 0) {
        mla_uint16_t extra_val = match_len - __mla_deflate_length_base[len_idx];
        __mla_deflate_bit_writer_write(writer, output, extra_val, extra);
    }

    // Emit distance code (fixed: 5 bits, reversed)
    mla_uint16_t dist_idx = __mla_deflate_dist_code(match_dist);
    __mla_deflate_bit_writer_write_reversed(writer, output, (mla_uint32_t)dist_idx, 5);

    // Emit distance extra bits
    mla_uint8_t dist_extra = __mla_deflate_dist_extra[dist_idx];
    if (dist_extra > 0) {
        mla_uint16_t dist_extra_val = match_dist - __mla_deflate_dist_base[dist_idx];
        __mla_deflate_bit_writer_write(writer, output, dist_extra_val, dist_extra);
    }
}

// Emit end-of-block marker
static void __mla_deflate_emit_end_of_block(__mla_deflate_bit_writer_t &writer, mla_stream_output_t &output) {
    mla_uint16_t code = __mla_deflate_fixed_tables.lit_code[mla_deflate_end_of_block];
    mla_uint8_t length = __mla_deflate_fixed_tables.lit_length[mla_deflate_end_of_block];
    __mla_deflate_bit_writer_write_reversed(writer, output, code, length);
}

// Find the best match in the window using hash chains
static mla_bool_t __mla_deflate_find_match(__mla_deflate_compress_state_t &state,
                                           const mla_byte_t *data, mla_size_t data_len, mla_size_t pos,
                                           mla_uint16_t &best_len, mla_uint16_t &best_dist) {
    best_len = 0;
    best_dist = 0;

    if (data_len - pos < mla_deflate_min_match) return false;
    if (state.window_filled < mla_deflate_min_match) return false;

    mla_uint16_t hash = __mla_deflate_hash3(data + pos);
    mla_uint16_t chain = state.hash_head[hash];
    mla_uint16_t chain_limit = mla_deflate_chain_limit;

    while (chain != 0xFFFF && chain_limit > 0) {
        chain_limit--;

        // Calculate the distance from current window position to chain entry
        mla_size_t chain_pos = chain;
        mla_size_t cur_win_pos = state.window_pos;

        mla_size_t dist;
        if (cur_win_pos >= chain_pos) {
            dist = cur_win_pos - chain_pos;
        } else {
            dist = mla_deflate_window_size - chain_pos + cur_win_pos;
        }

        if (dist == 0 || dist > state.window_filled || dist > mla_deflate_window_size) {
            break;
        }

        // Compare bytes
        mla_uint16_t match_len = 0;
        mla_size_t max_len = data_len - pos;
        if (max_len > mla_deflate_max_match) max_len = mla_deflate_max_match;

        for (mla_size_t j = 0; j < max_len; j++) {
            mla_size_t win_idx = (mla_deflate_window_size + state.window_pos - dist + j) & mla_deflate_window_mask;
            if (state.window[win_idx] == data[pos + j]) {
                match_len++;
            } else {
                break;
            }
        }

        if (match_len >= mla_deflate_min_match && match_len > best_len) {
            best_len = match_len;
            best_dist = (mla_uint16_t)dist;
            if (best_len >= mla_deflate_max_match) break;
        }

        chain = state.hash_prev[chain];
    }

    return best_len >= mla_deflate_min_match;
}

// Add bytes to the sliding window and update hash chains
static void __mla_deflate_window_add(__mla_deflate_compress_state_t &state, const mla_byte_t *data, mla_size_t length) {
    for (mla_size_t i = 0; i < length; i++) {
        state.window[state.window_pos] = data[i];

        // Update hash chains if we have at least 3 bytes in the window to hash
        if (state.window_filled >= 2 && i >= 2) {
            mla_uint16_t hash = __mla_deflate_hash3(data + i - 2);
            mla_size_t pos = (state.window_pos - 2) & mla_deflate_window_mask;
            state.hash_prev[pos] = state.hash_head[hash];
            state.hash_head[hash] = (mla_uint16_t)pos;
        } else if (state.window_filled >= 2) {
            // Use window data for the hash
            mla_byte_t hash_bytes[3];
            hash_bytes[0] = state.window[(mla_deflate_window_size + state.window_pos - 2) & mla_deflate_window_mask];
            hash_bytes[1] = state.window[(mla_deflate_window_size + state.window_pos - 1) & mla_deflate_window_mask];
            hash_bytes[2] = data[i];
            mla_uint16_t hash = __mla_deflate_hash3(hash_bytes);
            mla_size_t pos = (mla_deflate_window_size + state.window_pos - 2) & mla_deflate_window_mask;
            state.hash_prev[pos] = state.hash_head[hash];
            state.hash_head[hash] = (mla_uint16_t)pos;
        }

        state.window_pos = (state.window_pos + 1) & mla_deflate_window_mask;
        if (state.window_filled < mla_deflate_window_size) {
            state.window_filled++;
        }
    }
}

// Compress a block of data using LZ77 + fixed Huffman
static void __mla_deflate_compress_block(__mla_deflate_compress_state_t &state, const mla_byte_t *data, mla_size_t length, mla_bool_t is_final) {

    if (!state.block_started) {
        // Write block header: BFINAL (1 bit) + BTYPE (2 bits) = fixed Huffman (01)
        __mla_deflate_bit_writer_write(state.writer, state.base_output, is_final ? 1u : 0u, 1);
        __mla_deflate_bit_writer_write(state.writer, state.base_output, mla_deflate_block_fixed, 2);
        state.block_started = true;
    }

    mla_size_t pos = 0;
    while (pos < length) {
        mla_uint16_t best_len = 0;
        mla_uint16_t best_dist = 0;

        if (__mla_deflate_find_match(state, data, length, pos, best_len, best_dist)) {
            __mla_deflate_emit_match(state.writer, state.base_output, best_len, best_dist);
            __mla_deflate_window_add(state, data + pos, best_len);
            pos += best_len;
        } else {
            __mla_deflate_emit_literal(state.writer, state.base_output, data[pos]);
            __mla_deflate_window_add(state, data + pos, 1);
            pos++;
        }
    }
}

///////////////////////////////////////////////////////////////////
/// Compression Output Wrapper
///////////////////////////////////////////////////////////////////

mla_user_data_id_init(__mla_stream_deflate_compress_data_name)

static mla_size_t __mla_stream_deflate_compress_write(mla_stream_output_t &output, mla_size_t offset, mla_size_t length, const mla_byte_t *buffer) {
    __mla_deflate_compress_state_t *state = mla_user_data_get_pointer<__mla_deflate_compress_state_t>(output.userdata, __mla_stream_deflate_compress_data_name);

    if (state == nullptr || buffer == nullptr || state->finished) {
        return 0;
    }

    __mla_deflate_build_fixed_tables();

    const mla_byte_t *input_data = buffer + offset;

    if (!__mla_deflate_compress_ensure_container_header(*state)) {
        return 0;
    }

    if (state->mode == mla_deflate_mode_zlib && length > 0) {
        state->adler32 = __mla_deflate_adler32_update(state->adler32, input_data, length);
    }

    if (state->mode == mla_deflate_mode_gzip && length > 0) {
        state->crc32 = __mla_deflate_crc32_update(state->crc32, input_data, length);
        state->input_size += (mla_uint32_t)length;
    }

    __mla_deflate_compress_block(*state, input_data, length, false);

    if (state->writer.error) {
        return 0;
    }

    return length;
}

static mla_size_t __mla_stream_deflate_compress_available_bytes(mla_stream_output_t &output) {
    __mla_deflate_compress_state_t *state = mla_user_data_get_pointer<__mla_deflate_compress_state_t>(output.userdata, __mla_stream_deflate_compress_data_name);

    if (state == nullptr || state->finished) {
        return 0;
    }

    return mla_size_max;
}

mla_stream_output_t mla_stream_output_deflate_compress_wrapper(mla_stream_output_t &output) {
    return mla_stream_output_deflate_compress_wrapper(output, mla_deflate_mode_raw);
}

mla_stream_output_t mla_stream_output_deflate_compress_wrapper(mla_stream_output_t &output, mla_deflate_mode_t mode) {
    if (output.write == nullptr) {
        return mla_stream_noop_output();
    }

    __mla_deflate_build_fixed_tables();

    __mla_deflate_compress_state_t *state = static_cast<__mla_deflate_compress_state_t *>(mla_platform_malloc(sizeof(__mla_deflate_compress_state_t)));

    if (state == nullptr) {
        return mla_stream_noop_output();
    }

    mla_memset(state, 0, sizeof(__mla_deflate_compress_state_t));
    state->base_output = output;
    __mla_deflate_bit_writer_init(state->writer);
    state->block_started = false;
    state->finished = false;
    state->window_pos = 0;
    state->window_filled = 0;
    state->adler32 = 1u;
    state->crc32 = 0u;
    state->input_size = 0u;
    state->container_header_written = false;
    state->mode = mode;

    // Initialize hash heads to 0xFFFF (no entry)
    mla_memset(state->hash_head, 0xFF, sizeof(state->hash_head));
    mla_memset(state->hash_prev, 0xFF, sizeof(state->hash_prev));

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<__mla_deflate_compress_state_t, __mla_deflate_compress_state_initializer>(user_data, __mla_stream_deflate_compress_data_name, state);

    return {
        user_data,
        __mla_stream_deflate_compress_write,
        __mla_stream_deflate_compress_available_bytes
    };
}

mla_bool_t mla_stream_output_deflate_finish(mla_stream_output_t &output) {
    __mla_deflate_compress_state_t *state = mla_user_data_get_pointer<__mla_deflate_compress_state_t>(output.userdata, __mla_stream_deflate_compress_data_name);

    if (state == nullptr || state->finished) {
        return false;
    }

    __mla_deflate_build_fixed_tables();

    if (!__mla_deflate_compress_ensure_container_header(*state)) {
        return false;
    }

    // Step 1 – close the in-progress Huffman block (if any).
    if (state->block_started) {
        __mla_deflate_emit_end_of_block(state->writer, state->base_output);
        state->block_started = false;
    }


    if (state->mode == mla_deflate_mode_raw_websocket) {
        // RFC 7692 permessage-deflate expects the sender to produce the same
        // byte sequence as a Z_SYNC_FLUSH, then strip the trailing 4-byte
        // LEN/NLEN (00 00 FF FF).  Z_SYNC_FLUSH emits an empty stored block
        // with BFINAL=0 (not BFINAL=1).  The receiver re-appends the 4 bytes
        // and feeds the result to inflate with Z_SYNC_FLUSH, which expects
        // Z_OK – not Z_STREAM_END.  Using BFINAL=1 here would cause standard
        // clients (e.g. Node.js ws library) to receive Z_STREAM_END, which can
        // corrupt the inflate stream state and produce garbled output.
        __mla_deflate_bit_writer_write(state->writer, state->base_output, 0x00, 3);

        // Byte-align so the kept prefix exactly matches a Z_SYNC_FLUSH with
        // only the last four LEN/NLEN bytes removed.
        __mla_deflate_bit_writer_align(state->writer, state->base_output);
    } else {

        // Step 3 – write the empty stored-block header byte.
        //   Bit layout: BFINAL=1 (bit 0), BTYPE=00 (bits 1-2), padding=00000 (bits 3-7)
        //   → byte value 0x01.
        __mla_deflate_bit_writer_write(state->writer, state->base_output, 0x01, 3);

        // byte-align: flush any partially-accumulated bits with zero padding.
        __mla_deflate_bit_writer_align(state->writer, state->base_output);

        // Normal mode: complete the empty stored block with LEN=0 and NLEN=0xFFFF.
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, 0x00); // LEN  low
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, 0x00); // LEN  high
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, 0xFF); // NLEN low
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, 0xFF); // NLEN high
    }

    if (state->mode == mla_deflate_mode_zlib) {
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)((state->adler32 >> 24) & 0xFFu));
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)((state->adler32 >> 16) & 0xFFu));
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)((state->adler32 >> 8) & 0xFFu));
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)(state->adler32 & 0xFFu));
    }

    if (state->mode == mla_deflate_mode_gzip) {
        // CRC32 – little-endian (RFC 1952 section 2.3.1)
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)(state->crc32 & 0xFFu));
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)((state->crc32 >> 8) & 0xFFu));
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)((state->crc32 >> 16) & 0xFFu));
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)((state->crc32 >> 24) & 0xFFu));
        // ISIZE – little-endian (original input size mod 2^32)
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)(state->input_size & 0xFFu));
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)((state->input_size >> 8) & 0xFFu));
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)((state->input_size >> 16) & 0xFFu));
        __mla_deflate_bit_writer_put_byte(state->writer, state->base_output, (mla_byte_t)((state->input_size >> 24) & 0xFFu));
    }

    __mla_deflate_bit_writer_flush_buffer(state->writer, state->base_output);

    state->finished = true;

    return !state->writer.error;
}

///////////////////////////////////////////////////////////////////
/// Decompression Input Wrapper
///////////////////////////////////////////////////////////////////

enum __mla_deflate_container_mode_t : mla_uint8_t {
    __mla_deflate_container_mode_raw,
    __mla_deflate_container_mode_zlib,
    __mla_deflate_container_mode_gzip
};

struct __mla_deflate_decompress_state_t {
    mla_stream_input_t base_input;
    __mla_deflate_bit_reader_t reader;

    // Output window (circular buffer)
    mla_byte_t window[mla_deflate_window_size];
    mla_size_t window_pos;

    // Decoded output buffer
    mla_bytes_t output_buf;
    mla_size_t output_buf_pos;   // read position
    mla_size_t output_buf_len;   // amount of decoded data in buffer

    // Huffman tables for current block
    __mla_deflate_huffman_t<mla_deflate_fixed_lit_codes> lit_table;
    __mla_deflate_huffman_t<mla_deflate_max_dist_codes> dist_table;

    // Block state
    mla_bool_t block_final;
    mla_bool_t block_active;
    mla_uint8_t block_type;
    mla_size_t uncompressed_remaining; // for uncompressed blocks

    // Pending back-reference copy: set when a match copy was interrupted by a full
    // output buffer so it can be resumed on the next fill call.
    mla_uint16_t pending_match_remaining; // bytes still to copy from the back-reference
    mla_uint16_t pending_match_dist;      // distance of the interrupted back-reference

    mla_uint32_t adler32;
    mla_uint32_t crc32;
    mla_uint32_t decompressed_size;
    __mla_deflate_container_mode_t container_mode;
    mla_bool_t finished;
    mla_bool_t error;
};

struct __mla_deflate_decompress_state_initializer {

    static __mla_deflate_decompress_state_t init() {

        __mla_deflate_decompress_state_t result = {
            mla_stream_noop_input(),       // base_input
            {0, 0, {}, 0, 0, false}, // reader
            {}, 0,                         // window, window_pos
            mla_bytes_empty(), 0, 0,             // output_buf, output_buf_size, output_buf_pos, output_buf_len
            __mla_deflate_huffman_empty<mla_deflate_fixed_lit_codes>(),                   // lit_table
            __mla_deflate_huffman_empty<mla_deflate_max_dist_codes>(),                   // dist_table
            false, false, 0, 0,           // block_final, block_active, block_type, uncompressed_remaining
            0, 0,                          // pending_match_remaining, pending_match_dist
            1u, 0u, 0u,                   // adler32, crc32, decompressed_size
            __mla_deflate_container_mode_raw, // container_mode
            false, false                   // finished, error
        };
        return result;
    }
};

static mla_bool_t __mla_deflate_decompress_consume_zlib_trailer(__mla_deflate_decompress_state_t &state) {
    __mla_deflate_bit_reader_align(state.reader);

    mla_byte_t trailer[4];
    for (mla_size_t i = 0; i < 4; i++) {
        if (!__mla_deflate_bit_reader_read_byte(state.reader, state.base_input, trailer[i])) {
            state.error = true;
            return false;
        }
    }

    mla_uint32_t expected =
        ((mla_uint32_t)trailer[0] << 24) |
        ((mla_uint32_t)trailer[1] << 16) |
        ((mla_uint32_t)trailer[2] << 8) |
        (mla_uint32_t)trailer[3];

    if (expected != state.adler32) {
        state.error = true;
        return false;
    }

    return true;
}

static mla_bool_t __mla_deflate_decompress_consume_gzip_trailer(__mla_deflate_decompress_state_t &state) {
    __mla_deflate_bit_reader_align(state.reader);

    // CRC32 – 4 bytes, little-endian
    mla_byte_t trailer[8];
    for (mla_size_t i = 0; i < 8; i++) {
        if (!__mla_deflate_bit_reader_read_byte(state.reader, state.base_input, trailer[i])) {
            state.error = true;
            return false;
        }
    }

    mla_uint32_t expected_crc =
        (mla_uint32_t)trailer[0] |
        ((mla_uint32_t)trailer[1] << 8) |
        ((mla_uint32_t)trailer[2] << 16) |
        ((mla_uint32_t)trailer[3] << 24);

    if (expected_crc != state.crc32) {
        state.error = true;
        return false;
    }

    // ISIZE – 4 bytes, little-endian (original size mod 2^32)
    mla_uint32_t expected_size =
        (mla_uint32_t)trailer[4] |
        ((mla_uint32_t)trailer[5] << 8) |
        ((mla_uint32_t)trailer[6] << 16) |
        ((mla_uint32_t)trailer[7] << 24);

    if (expected_size != state.decompressed_size) {
        state.error = true;
        return false;
    }

    return true;
}

mla_user_data_id_init(__mla_stream_deflate_decompress_data_name)


static __mla_deflate_huffman_t<mla_deflate_fixed_lit_codes> __mla_deflate_fixed_lit_decode_table;
static __mla_deflate_huffman_t<mla_deflate_max_dist_codes> __mla_deflate_fixed_dist_decode_table;
static mla_bool_t __mla_deflate_fixed_decode_tables_initialized = false;

static void __mla_deflate_build_fixed_decode_tables_once() {
    if (__mla_deflate_fixed_decode_tables_initialized)
        return;

    mla_uint8_t lengths[mla_deflate_fixed_lit_codes];
    mla_uint16_t i;

    // Literal/Length per RFC 1951 3.2.6: 288 entries (0..287)
    // 0..143 = 8 bits, 144..255 = 9 bits, 256..279 = 7 bits, 280..287 = 8 bits
    for (i = 0; i <= 143; i++) lengths[i] = 8;
    for (i = 144; i <= 255; i++) lengths[i] = 9;
    for (i = 256; i <= 279; i++) lengths[i] = 7;
    for (i = 280; i <= 287; i++) lengths[i] = 8;

    __mla_deflate_huffman_build(__mla_deflate_fixed_lit_decode_table, lengths, mla_deflate_fixed_lit_codes);

    // Distance: all 5 bits
    mla_uint8_t dist_lengths[mla_deflate_max_dist_codes];
    for (i = 0; i < mla_deflate_max_dist_codes; i++)
        dist_lengths[i] = 5;

    __mla_deflate_huffman_build(__mla_deflate_fixed_dist_decode_table, dist_lengths, mla_deflate_max_dist_codes);
    __mla_deflate_fixed_decode_tables_initialized = true;
}

// Decode dynamic Huffman tables from the stream
static mla_bool_t __mla_deflate_decode_dynamic_tables(__mla_deflate_decompress_state_t &state) {
    __mla_deflate_bit_reader_t &reader = state.reader;

    mla_uint32_t hlit = __mla_deflate_bit_reader_read(reader, state.base_input, 5) + 257;
    mla_uint32_t hdist = __mla_deflate_bit_reader_read(reader, state.base_input, 5) + 1;
    mla_uint32_t hclen = __mla_deflate_bit_reader_read(reader, state.base_input, 4) + 4;

    if (reader.error) return false;

    if (hlit > mla_deflate_max_lit_codes || hdist > mla_deflate_max_dist_codes) {
        state.error = true;
        return false;
    }

    // Read code length code lengths
    mla_uint8_t cl_lengths[mla_deflate_max_cl_codes];
    mla_memset(cl_lengths, 0, sizeof(cl_lengths));

    for (mla_uint32_t i = 0; i < hclen; i++) {
        cl_lengths[__mla_deflate_cl_order[i]] = (mla_uint8_t)__mla_deflate_bit_reader_read(reader, state.base_input, 3);
        if (reader.error) return false;
    }

    // Build the code length Huffman table
    __mla_deflate_huffman_t<mla_deflate_max_cl_codes> cl_table = __mla_deflate_huffman_empty<mla_deflate_max_cl_codes>();
    __mla_deflate_huffman_init(cl_table);
    if (!__mla_deflate_huffman_build(cl_table, cl_lengths, mla_deflate_max_cl_codes)) {
        state.error = true;
        return false;
    }

    // Decode literal/length and distance code lengths
    mla_uint8_t lengths[mla_deflate_max_codes];
    mla_memset(lengths, 0, sizeof(lengths));
    mla_uint32_t total = hlit + hdist;
    mla_uint32_t idx = 0;

    while (idx < total) {
        mla_int32_t symbol = __mla_deflate_huffman_decode(reader, state.base_input, cl_table);
        if (symbol < 0) {
            state.error = true;
            return false;
        }

        if (symbol < 16) {
            lengths[idx++] = (mla_uint8_t)symbol;
        } else if (symbol == 16) {
            // Repeat previous length 3..6 times
            if (idx == 0) {
                state.error = true;
                return false;
            }
            mla_uint32_t repeat = __mla_deflate_bit_reader_read(reader, state.base_input, 2) + 3;
            if (reader.error) return false;
            mla_uint8_t prev = lengths[idx - 1];
            for (mla_uint32_t r = 0; r < repeat && idx < total; r++) {
                lengths[idx++] = prev;
            }
        } else if (symbol == 17) {
            // Repeat 0 for 3..10 times
            mla_uint32_t repeat = __mla_deflate_bit_reader_read(reader, state.base_input, 3) + 3;
            if (reader.error) return false;
            for (mla_uint32_t r = 0; r < repeat && idx < total; r++) {
                lengths[idx++] = 0;
            }
        } else if (symbol == 18) {
            // Repeat 0 for 11..138 times
            mla_uint32_t repeat = __mla_deflate_bit_reader_read(reader, state.base_input, 7) + 11;
            if (reader.error) return false;
            for (mla_uint32_t r = 0; r < repeat && idx < total; r++) {
                lengths[idx++] = 0;
            }
        } else {
            state.error = true;
            return false;
        }
    }

    // Build literal/length and distance tables
    if (!__mla_deflate_huffman_build(state.lit_table, lengths, (mla_uint16_t)hlit)) {
        state.error = true;
        return false;
    }
    if (!__mla_deflate_huffman_build(state.dist_table, lengths + hlit, (mla_uint16_t)hdist)) {
        state.error = true;
        return false;
    }

    return true;
}

// Decode data from a Huffman-coded block into the output buffer
static mla_bool_t __mla_deflate_decode_huffman_block(__mla_deflate_decompress_state_t &state) {
    __mla_deflate_bit_reader_t &reader = state.reader;

    mla_size_t output_buf_size = mla_bytes_length(state.output_buf);
    mla_byte_t* output_buf = mla_bytes_get_data_for_writing(state.output_buf);

    while (state.output_buf_len < output_buf_size) {

        // Resume a back-reference copy that was interrupted by a full output buffer
        // on the previous fill call. Must be drained before decoding new symbols.
        if (state.pending_match_remaining > 0) {
            while (state.pending_match_remaining > 0 && state.output_buf_len < output_buf_size) {
                mla_size_t src_pos = (mla_deflate_window_size + state.window_pos - (mla_size_t)state.pending_match_dist) & mla_deflate_window_mask;
                mla_byte_t byte_val = state.window[src_pos];
                output_buf[state.output_buf_len++] = byte_val;
                state.window[state.window_pos] = byte_val;
                state.window_pos = (state.window_pos + 1) & mla_deflate_window_mask;
                state.adler32 = __mla_deflate_adler32_update_byte(state.adler32, byte_val);
                state.crc32 = __mla_deflate_crc32_update_byte(state.crc32, byte_val);
                state.decompressed_size++;
                state.pending_match_remaining--;
            }
            continue; // re-check outer while condition before decoding a new symbol
        }

        mla_int32_t symbol = __mla_deflate_huffman_decode(reader, state.base_input, state.lit_table);

        if (symbol < 0) {
            state.error = true;
            return false;
        }

        if (symbol < 256) {
            // Literal byte
            output_buf[state.output_buf_len++] = (mla_byte_t)symbol;
            state.window[state.window_pos] = (mla_byte_t)symbol;
            state.window_pos = (state.window_pos + 1) & mla_deflate_window_mask;
            state.adler32 = __mla_deflate_adler32_update_byte(state.adler32, (mla_byte_t)symbol);
            state.crc32 = __mla_deflate_crc32_update_byte(state.crc32, (mla_byte_t)symbol);
            state.decompressed_size++;
        } else if (symbol == mla_deflate_end_of_block) {
            // End of block
            state.block_active = false;
            return true;
        } else {
            // Length/distance pair
            mla_uint16_t len_idx = (mla_uint16_t)(symbol - 257);
            if (len_idx >= 29) {
                state.error = true;
                return false;
            }
            mla_uint16_t match_len = __mla_deflate_length_base[len_idx];
            mla_uint8_t extra = __mla_deflate_length_extra[len_idx];
            if (extra > 0) {
                match_len += (mla_uint16_t)__mla_deflate_bit_reader_read(reader, state.base_input, extra);
                if (reader.error) {
                    state.error = true;
                    return false;
                }
            }

            // Decode distance
            mla_int32_t dist_symbol = __mla_deflate_huffman_decode(reader, state.base_input, state.dist_table);
            if (dist_symbol < 0 || dist_symbol >= mla_deflate_max_dist_codes) {
                state.error = true;
                return false;
            }
            mla_uint16_t match_dist = __mla_deflate_dist_base[dist_symbol];
            mla_uint8_t dist_extra = __mla_deflate_dist_extra[dist_symbol];
            if (dist_extra > 0) {
                match_dist += (mla_uint16_t)__mla_deflate_bit_reader_read(reader, state.base_input, dist_extra);
                if (reader.error) {
                    state.error = true;
                    return false;
                }
            }

            // Copy from window
            for (mla_uint16_t i = 0; i < match_len; i++) {
                mla_size_t src_pos = (mla_deflate_window_size + state.window_pos - match_dist) & mla_deflate_window_mask;
                mla_byte_t byte_val = state.window[src_pos];
                output_buf[state.output_buf_len++] = byte_val;
                state.window[state.window_pos] = byte_val;
                state.window_pos = (state.window_pos + 1) & mla_deflate_window_mask;
                state.adler32 = __mla_deflate_adler32_update_byte(state.adler32, byte_val);
                state.crc32 = __mla_deflate_crc32_update_byte(state.crc32, byte_val);
                state.decompressed_size++;

                if (state.output_buf_len >= output_buf_size) {
                    // Output buffer full mid-match — save the remainder so the next
                    // fill call can resume from here without losing any bytes.
                    state.pending_match_remaining = match_len - (mla_uint16_t)(i + 1);
                    state.pending_match_dist      = match_dist;
                    break;
                }
            }
        }
    }

    return true;
}

// Decode uncompressed block data
static mla_bool_t __mla_deflate_decode_uncompressed_block(__mla_deflate_decompress_state_t &state) {

    mla_size_t output_buf_size = mla_bytes_length(state.output_buf);
    mla_byte_t* output_buf = mla_bytes_get_data_for_writing(state.output_buf);

    while (state.uncompressed_remaining > 0 && state.output_buf_len < output_buf_size) {
        mla_byte_t byte_val = 0;
        if (!__mla_deflate_bit_reader_read_byte(state.reader, state.base_input, byte_val)) {
            return false;
        }
        output_buf[state.output_buf_len++] = byte_val;
        state.window[state.window_pos] = byte_val;
        state.window_pos = (state.window_pos + 1) & mla_deflate_window_mask;
        state.adler32 = __mla_deflate_adler32_update_byte(state.adler32, byte_val);
        state.crc32 = __mla_deflate_crc32_update_byte(state.crc32, byte_val);
        state.decompressed_size++;
        state.uncompressed_remaining--;
    }

    if (state.uncompressed_remaining == 0) {
        state.block_active = false;
    }

    return true;
}

// Decode compressed data into the output buffer
static mla_bool_t __mla_deflate_decompress_fill(__mla_deflate_decompress_state_t &state) {

    mla_size_t output_buf_size = mla_bytes_length(state.output_buf);

    state.output_buf_pos = 0;
    state.output_buf_len = 0;

    while (state.output_buf_len < output_buf_size && !state.finished && !state.error) {

        if (!state.block_active) {
            // Read block header
            mla_uint32_t bfinal = __mla_deflate_bit_reader_read(state.reader, state.base_input, 1);
            mla_uint32_t btype = __mla_deflate_bit_reader_read(state.reader, state.base_input, 2);

            if (state.reader.error) {
                state.error = true;
                return false;
            }

            state.block_final = (bfinal == 1);
            state.block_type = (mla_uint8_t)btype;
            state.block_active = true;

            if (btype == mla_deflate_block_uncompressed) {
                // Align to byte boundary
                __mla_deflate_bit_reader_align(state.reader);

                // Read LEN and NLEN
                mla_uint32_t len = __mla_deflate_bit_reader_read(state.reader, state.base_input, 16);
                mla_uint32_t nlen = __mla_deflate_bit_reader_read(state.reader, state.base_input, 16);

                if (state.reader.error) {
                    state.error = true;
                    return false;
                }

                if ((len ^ nlen) != 0xFFFF) {
                    state.error = true;
                    return false;
                }

                state.uncompressed_remaining = len;
            } else if (btype == mla_deflate_block_fixed) {
                __mla_deflate_build_fixed_decode_tables_once();
                state.lit_table = __mla_deflate_fixed_lit_decode_table;
                state.dist_table = __mla_deflate_fixed_dist_decode_table;
            } else if (btype == mla_deflate_block_dynamic) {
                if (!__mla_deflate_decode_dynamic_tables(state)) {
                    return false;
                }
            } else {
                state.error = true;
                return false;
            }
        }

        if (state.block_type == mla_deflate_block_uncompressed) {
            if (!__mla_deflate_decode_uncompressed_block(state)) {
                return false;
            }
        } else {
            if (!__mla_deflate_decode_huffman_block(state)) {
                return false;
            }
        }

        if (!state.block_active && state.block_final) {
            if (state.container_mode == __mla_deflate_container_mode_zlib) {
                if (!__mla_deflate_decompress_consume_zlib_trailer(state)) {
                    return false;
                }
            } else if (state.container_mode == __mla_deflate_container_mode_gzip) {
                if (!__mla_deflate_decompress_consume_gzip_trailer(state)) {
                    return false;
                }
            }
            state.finished = true;
        }
    }

    return true;
}

static mla_size_t __mla_stream_deflate_decompress_read(mla_stream_input_t &input, mla_size_t offset, mla_size_t length, mla_byte_t *buffer) {
    __mla_deflate_decompress_state_t *state = mla_user_data_get_pointer<__mla_deflate_decompress_state_t>(input.userdata, __mla_stream_deflate_decompress_data_name);

    if (state == nullptr || buffer == nullptr) {
        return 0;
    }

    mla_size_t total_read = 0;

    mla_byte_t* output_buf = mla_bytes_get_data_for_writing(state->output_buf);

    while (total_read < length) {
        // If there's data in the output buffer, copy it
        if (state->output_buf_pos < state->output_buf_len) {
            mla_size_t available = state->output_buf_len - state->output_buf_pos;
            mla_size_t to_copy = length - total_read;
            if (to_copy > available) to_copy = available;
            mla_memcpy(buffer + offset + total_read, output_buf + state->output_buf_pos, to_copy);
            state->output_buf_pos += to_copy;
            total_read += to_copy;
        } else {
            // Need more data
            if (state->finished || state->error) {
                break;
            }

            // Ignore the boolean return: if fill decoded some data before encountering
            // an error or EOF (e.g. the WebSocket empty stored-block tail), we still want
            // to return those bytes. The copy branch above will drain output_buf on the
            // next iteration, and the error/finished guard will stop the loop afterward.
            __mla_deflate_decompress_fill(*state);

            if (state->output_buf_len == 0) {
                break;
            }
        }
    }

    return total_read;
}

static mla_size_t __mla_stream_deflate_decompress_remaining_bytes(mla_stream_input_t &input) {
    __mla_deflate_decompress_state_t *state = mla_user_data_get_pointer<__mla_deflate_decompress_state_t>(input.userdata, __mla_stream_deflate_decompress_data_name);

    if (state == nullptr) {
        return 0;
    }

    if (state->finished && state->output_buf_pos >= state->output_buf_len) {
        return 0;
    }

    if (state->error) {
        return 0;
    }

    // We have buffered data or stream is not finished
    if (state->output_buf_pos < state->output_buf_len) {
        return state->output_buf_len - state->output_buf_pos;
    }

    return mla_size_max; // Data available but unknown size
}

mla_stream_input_t mla_stream_input_deflate_decompress_wrapper(mla_stream_input_t &input) {
    if (input.read == nullptr) {
        return mla_stream_noop_input();
    }

    __mla_deflate_decompress_state_t *state = static_cast<__mla_deflate_decompress_state_t *>(mla_platform_malloc(sizeof(__mla_deflate_decompress_state_t)));

    if (state == nullptr) {
        return mla_stream_noop_input();
    }

    mla_memset(state, 0, sizeof(__mla_deflate_decompress_state_t));
    state->base_input = input;
    __mla_deflate_bit_reader_init(state->reader);
    state->window_pos = 0;
    state->block_active = false;
    state->block_final = false;
    state->adler32 = 1u;
    state->crc32 = 0u;
    state->decompressed_size = 0u;
    state->container_mode = __mla_deflate_container_mode_raw;
    state->finished = false;
    state->error = false;

    mla_byte_t prefix[2];
    mla_size_t prefix_length = 0;
    for (; prefix_length < 2; prefix_length++) {
        mla_size_t read_count = state->base_input.read(state->base_input, 0, 1, prefix + prefix_length);
        if (read_count == 0) {
            break;
        }
    }

    if (prefix_length == 2 && prefix[0] == 0x1F && prefix[1] == 0x8B) {
        // Gzip header detected (RFC 1952)
        mla_byte_t gzip_hdr[8]; // remaining 8 bytes of the 10-byte fixed header
        mla_size_t hdr_read = 0;
        for (; hdr_read < 8; hdr_read++) {
            mla_size_t r = state->base_input.read(state->base_input, 0, 1, gzip_hdr + hdr_read);
            if (r == 0) { state->error = true; break; }
        }

        if (!state->error && gzip_hdr[0] != 8) {
            // CM must be 8 (deflate)
            state->error = true;
        }

        if (!state->error) {
            mla_uint8_t flg = gzip_hdr[1];

            // FEXTRA (bit 2)
            if (flg & 0x04u) {
                mla_byte_t xlen_buf[2];
                if (state->base_input.read(state->base_input, 0, 1, xlen_buf) == 0 ||
                    state->base_input.read(state->base_input, 0, 1, xlen_buf + 1) == 0) {
                    state->error = true;
                } else {
                    mla_uint16_t xlen = (mla_uint16_t)xlen_buf[0] | ((mla_uint16_t)xlen_buf[1] << 8);
                    for (mla_uint16_t i = 0; i < xlen && !state->error; i++) {
                        mla_byte_t discard;
                        if (state->base_input.read(state->base_input, 0, 1, &discard) == 0) {
                            state->error = true;
                        }
                    }
                }
            }

            // FNAME (bit 3) – zero-terminated
            if (!state->error && (flg & 0x08u)) {
                mla_byte_t ch;
                do {
                    if (state->base_input.read(state->base_input, 0, 1, &ch) == 0) {
                        state->error = true; break;
                    }
                } while (ch != 0 && !state->error);
            }

            // FCOMMENT (bit 4) – zero-terminated
            if (!state->error && (flg & 0x10u)) {
                mla_byte_t ch;
                do {
                    if (state->base_input.read(state->base_input, 0, 1, &ch) == 0) {
                        state->error = true; break;
                    }
                } while (ch != 0 && !state->error);
            }

            // FHCRC (bit 1) – 2-byte header CRC
            if (!state->error && (flg & 0x02u)) {
                mla_byte_t hcrc[2];
                if (state->base_input.read(state->base_input, 0, 1, hcrc) == 0 ||
                    state->base_input.read(state->base_input, 0, 1, hcrc + 1) == 0) {
                    state->error = true;
                }
            }
        }

        if (!state->error) {
            state->container_mode = __mla_deflate_container_mode_gzip;
        }
    } else if (prefix_length == 2 && __mla_deflate_is_zlib_header(prefix[0], prefix[1])) {
        mla_uint8_t flg = prefix[1];
        mla_size_t header_window_bits = (mla_size_t)((prefix[0] >> 4) + 8);

        if ((flg & 0x20u) != 0 || header_window_bits > __mla_deflate_window_bits_value()) {
            state->error = true;
        } else {
            state->container_mode = __mla_deflate_container_mode_zlib;
        }
    } else {
        state->reader.prefetched_length = (mla_uint8_t)prefix_length;
        state->reader.prefetched_position = 0;
        for (mla_size_t i = 0; i < prefix_length; i++) {
            state->reader.prefetched_bytes[i] = prefix[i];
        }
    }

    // Allocate output buffer
    mla_size_t buf_size = mla_deflate_compress_buf_size;
    state->output_buf = mla_bytes(buf_size);
    if (mla_bytes_is_empty(state->output_buf)) {
        mla_platform_free(state);
        return mla_stream_noop_input();
    }
    state->output_buf_pos = 0;
    state->output_buf_len = 0;
    state->pending_match_remaining = 0;
    state->pending_match_dist      = 0;

    __mla_deflate_huffman_init(state->lit_table);
    __mla_deflate_huffman_init(state->dist_table);

    mla_user_data_t user_data = mla_user_data_empty();
    mla_user_data_set_pointer_with_ownership<__mla_deflate_decompress_state_t, __mla_deflate_decompress_state_initializer>(user_data, __mla_stream_deflate_decompress_data_name, state);

    return {
        user_data,
        __mla_stream_deflate_decompress_read,
        __mla_stream_deflate_decompress_remaining_bytes
    };
}


mla_size_t mla_stream_input_deflate_decompressed_size_calculation(mla_stream_input_t &input) {

    // Calc the decompressed content size
    mla_stream_output_t size_decompression_stream = mla_stream_output_size_calculation();
    mla_stream_input_t deflate_stream = mla_stream_input_deflate_decompress_wrapper(input);
    mla_stream_copy(deflate_stream, size_decompression_stream);
    return mla_stream_output_size_calculation_get_size(size_decompression_stream);

}

mla_size_t mla_stream_input_deflate_compressed_size_calculation(mla_stream_input_t &input, mla_deflate_mode_t mode) {

    // Calc the compressed content size
    mla_stream_output_t size_compression_stream = mla_stream_output_size_calculation();
    mla_stream_output_t deflate_stream = mla_stream_output_deflate_compress_wrapper(size_compression_stream, mode);
    mla_stream_copy(input, deflate_stream);
    mla_stream_output_deflate_finish(deflate_stream);
    return mla_stream_output_size_calculation_get_size(size_compression_stream);

}

mla_size_t mla_stream_output_deflate_window_bits(mla_stream_output_t &output) {
    (void)output; // window bits depend only on the compile-time window size

    return __mla_deflate_window_bits_value();
}
