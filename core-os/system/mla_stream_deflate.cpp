//
// Created by copilot on 3/16/2026.
//

#include "mla_stream.h"
#include "../log/mla_logging.h"

///////////////////////////////////////////////////////////////////
/// DEFLATE Internal Constants and Structures (RFC 1951)
///////////////////////////////////////////////////////////////////

// DEFLATE block types
#define MLA_DEFLATE_BLOCK_UNCOMPRESSED  0
#define MLA_DEFLATE_BLOCK_FIXED         1
#define MLA_DEFLATE_BLOCK_DYNAMIC       2

// DEFLATE limits
#define MLA_DEFLATE_MAX_BITS            15
#define MLA_DEFLATE_MAX_LIT_CODES       286
#define MLA_DEFLATE_MAX_DIST_CODES      30
#define MLA_DEFLATE_MAX_CL_CODES        19
#define MLA_DEFLATE_MAX_CODES           (MLA_DEFLATE_MAX_LIT_CODES + MLA_DEFLATE_MAX_DIST_CODES)
#define MLA_DEFLATE_END_OF_BLOCK        256
#define MLA_DEFLATE_WINDOW_SIZE         32768
#define MLA_DEFLATE_WINDOW_MASK         (MLA_DEFLATE_WINDOW_SIZE - 1)
#define MLA_DEFLATE_MIN_MATCH           3
#define MLA_DEFLATE_MAX_MATCH           258
#define MLA_DEFLATE_HASH_BITS           15
#define MLA_DEFLATE_HASH_SIZE           (1 << MLA_DEFLATE_HASH_BITS)
#define MLA_DEFLATE_HASH_MASK           (MLA_DEFLATE_HASH_SIZE - 1)

// Compression output buffer size
#define MLA_DEFLATE_COMPRESS_BUF_SIZE   4096

// Code length alphabet order (RFC 1951 section 3.2.7)
static const mla_uint8_t __mla_deflate_cl_order[MLA_DEFLATE_MAX_CL_CODES] = {
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

struct __mla_deflate_huffman_t {
    mla_uint16_t counts[MLA_DEFLATE_MAX_BITS + 1]; // Number of symbols per bit length
    mla_uint16_t symbols[MLA_DEFLATE_MAX_CODES];   // Sorted symbols
    mla_uint16_t num_symbols;
};

static void __mla_deflate_huffman_init(__mla_deflate_huffman_t &table) {
    mla_memset(&table, 0, sizeof(__mla_deflate_huffman_t));
}

// Build a Huffman decode table from code lengths
static mla_bool_t __mla_deflate_huffman_build(__mla_deflate_huffman_t &table, const mla_uint8_t *lengths, mla_uint16_t num_symbols) {

    mla_memset(table.counts, 0, sizeof(table.counts));
    mla_memset(table.symbols, 0, sizeof(table.symbols));
    table.num_symbols = num_symbols;

    // Count the number of codes for each bit length
    for (mla_uint16_t i = 0; i < num_symbols; i++) {
        if (lengths[i] > MLA_DEFLATE_MAX_BITS) {
            return false;
        }
        table.counts[lengths[i]]++;
    }
    table.counts[0] = 0; // Codes of length 0 are not used

    // Compute offset table for sorting symbols
    mla_uint16_t offsets[MLA_DEFLATE_MAX_BITS + 1];
    offsets[0] = 0;
    offsets[1] = 0;
    for (mla_uint16_t i = 1; i < MLA_DEFLATE_MAX_BITS; i++) {
        offsets[i + 1] = offsets[i] + table.counts[i];
    }

    // Sort symbols by their code length, then by symbol value
    for (mla_uint16_t i = 0; i < num_symbols; i++) {
        if (lengths[i] != 0) {
            table.symbols[offsets[lengths[i]]++] = i;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////
/// Bit Reader (for decompression)
///////////////////////////////////////////////////////////////////

struct __mla_deflate_bit_reader_t {
    mla_stream_input_t *input;
    mla_uint32_t bit_buffer;
    mla_uint8_t bit_count;
    mla_bool_t error;
};

static void __mla_deflate_bit_reader_init(__mla_deflate_bit_reader_t &reader, mla_stream_input_t &input) {
    reader.input = &input;
    reader.bit_buffer = 0;
    reader.bit_count = 0;
    reader.error = false;
}

static mla_bool_t __mla_deflate_bit_reader_ensure(__mla_deflate_bit_reader_t &reader, mla_uint8_t bits_needed) {
    while (reader.bit_count < bits_needed) {
        mla_byte_t byte_val = 0;
        mla_size_t read_count = reader.input->read(*reader.input, 0, 1, &byte_val);
        if (read_count == 0) {
            reader.error = true;
            return false;
        }
        reader.bit_buffer |= ((mla_uint32_t)byte_val) << reader.bit_count;
        reader.bit_count += 8;
    }
    return true;
}

static mla_uint32_t __mla_deflate_bit_reader_read(__mla_deflate_bit_reader_t &reader, mla_uint8_t num_bits) {
    if (num_bits == 0) return 0;

    if (!__mla_deflate_bit_reader_ensure(reader, num_bits)) {
        return 0;
    }

    mla_uint32_t value = reader.bit_buffer & ((1u << num_bits) - 1);
    reader.bit_buffer >>= num_bits;
    reader.bit_count -= num_bits;
    return value;
}

// Decode a Huffman code from the bit stream
static mla_int32_t __mla_deflate_huffman_decode(__mla_deflate_bit_reader_t &reader, const __mla_deflate_huffman_t &table) {
    mla_uint32_t code = 0;
    mla_uint32_t first = 0;
    mla_uint32_t index = 0;

    for (mla_uint8_t len = 1; len <= MLA_DEFLATE_MAX_BITS; len++) {
        if (!__mla_deflate_bit_reader_ensure(reader, len)) {
            return -1;
        }

        // Read one bit
        code |= ((reader.bit_buffer >> (len - 1)) & 1) << (len - 1);

        // Get the reversed version of the code (DEFLATE stores bits LSB first)
        mla_uint32_t reversed = 0;
        for (mla_uint8_t b = 0; b < len; b++) {
            reversed = (reversed << 1) | ((code >> b) & 1);
        }

        mla_uint32_t count = table.counts[len];
        if (reversed - first < count) {
            // Consume the bits
            reader.bit_buffer >>= len;
            reader.bit_count -= len;
            return (mla_int32_t)table.symbols[index + (reversed - first)];
        }
        index += count;
        first = (first + count) << 1;
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

///////////////////////////////////////////////////////////////////
/// Bit Writer (for compression)
///////////////////////////////////////////////////////////////////

struct __mla_deflate_bit_writer_t {
    mla_stream_output_t *output;
    mla_byte_t buffer[MLA_DEFLATE_COMPRESS_BUF_SIZE];
    mla_size_t buffer_pos;
    mla_uint32_t bit_buffer;
    mla_uint8_t bit_count;
    mla_bool_t error;
};

static void __mla_deflate_bit_writer_init(__mla_deflate_bit_writer_t &writer, mla_stream_output_t &output) {
    writer.output = &output;
    writer.buffer_pos = 0;
    writer.bit_buffer = 0;
    writer.bit_count = 0;
    writer.error = false;
}

static void __mla_deflate_bit_writer_flush_buffer(__mla_deflate_bit_writer_t &writer) {
    if (writer.buffer_pos > 0) {
        mla_size_t written = writer.output->write(*writer.output, 0, writer.buffer_pos, writer.buffer);
        if (written != writer.buffer_pos) {
            writer.error = true;
        }
        writer.buffer_pos = 0;
    }
}

static void __mla_deflate_bit_writer_put_byte(__mla_deflate_bit_writer_t &writer, mla_byte_t byte_val) {
    writer.buffer[writer.buffer_pos++] = byte_val;
    if (writer.buffer_pos >= MLA_DEFLATE_COMPRESS_BUF_SIZE) {
        __mla_deflate_bit_writer_flush_buffer(writer);
    }
}

static void __mla_deflate_bit_writer_write(__mla_deflate_bit_writer_t &writer, mla_uint32_t value, mla_uint8_t num_bits) {
    writer.bit_buffer |= value << writer.bit_count;
    writer.bit_count += num_bits;

    while (writer.bit_count >= 8) {
        __mla_deflate_bit_writer_put_byte(writer, (mla_byte_t)(writer.bit_buffer & 0xFF));
        writer.bit_buffer >>= 8;
        writer.bit_count -= 8;
    }
}

// Write bits in reversed order (MSB first, for Huffman codes)
static void __mla_deflate_bit_writer_write_reversed(__mla_deflate_bit_writer_t &writer, mla_uint32_t code, mla_uint8_t length) {
    mla_uint32_t reversed = 0;
    for (mla_uint8_t i = 0; i < length; i++) {
        reversed = (reversed << 1) | (code & 1);
        code >>= 1;
    }
    __mla_deflate_bit_writer_write(writer, reversed, length);
}

static void __mla_deflate_bit_writer_align(__mla_deflate_bit_writer_t &writer) {
    if (writer.bit_count > 0) {
        __mla_deflate_bit_writer_put_byte(writer, (mla_byte_t)(writer.bit_buffer & 0xFF));
        writer.bit_buffer = 0;
        writer.bit_count = 0;
    }
}

static void __mla_deflate_bit_writer_finish(__mla_deflate_bit_writer_t &writer) {
    __mla_deflate_bit_writer_align(writer);
    __mla_deflate_bit_writer_flush_buffer(writer);
}

///////////////////////////////////////////////////////////////////
/// Fixed Huffman Tables (RFC 1951 section 3.2.6)
///////////////////////////////////////////////////////////////////

struct __mla_deflate_fixed_tables_t {
    mla_uint16_t lit_code[MLA_DEFLATE_MAX_LIT_CODES];
    mla_uint8_t lit_length[MLA_DEFLATE_MAX_LIT_CODES];
    mla_uint16_t dist_code[MLA_DEFLATE_MAX_DIST_CODES];
    mla_uint8_t dist_length[MLA_DEFLATE_MAX_DIST_CODES];
    mla_bool_t initialized;
};

static __mla_deflate_fixed_tables_t __mla_deflate_fixed_tables = { {}, {}, {}, {}, false };

static void __mla_deflate_build_fixed_tables() {

    if (__mla_deflate_fixed_tables.initialized) return;

    // Build literal/length code lengths per RFC 1951 3.2.6:
    //   0..143   -> 8 bits
    //   144..255 -> 9 bits
    //   256..279 -> 7 bits
    //   280..285 -> 8 bits
    mla_uint16_t i;
    for (i = 0; i <= 143; i++) __mla_deflate_fixed_tables.lit_length[i] = 8;
    for (i = 144; i <= 255; i++) __mla_deflate_fixed_tables.lit_length[i] = 9;
    for (i = 256; i <= 279; i++) __mla_deflate_fixed_tables.lit_length[i] = 7;
    for (i = 280; i <= 285; i++) __mla_deflate_fixed_tables.lit_length[i] = 8;

    // Build the actual Huffman codes from the lengths
    // Step 1: Count the number of codes for each bit length
    mla_uint16_t bl_count[MLA_DEFLATE_MAX_BITS + 1];
    mla_memset(bl_count, 0, sizeof(bl_count));
    for (i = 0; i < MLA_DEFLATE_MAX_LIT_CODES; i++) {
        bl_count[__mla_deflate_fixed_tables.lit_length[i]]++;
    }

    // Step 2: Find the numerical value of the smallest code for each code length
    mla_uint16_t next_code[MLA_DEFLATE_MAX_BITS + 1];
    mla_memset(next_code, 0, sizeof(next_code));
    mla_uint32_t code = 0;
    for (mla_uint8_t bits = 1; bits <= MLA_DEFLATE_MAX_BITS; bits++) {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = (mla_uint16_t)code;
    }

    // Step 3: Assign codes to symbols
    for (i = 0; i < MLA_DEFLATE_MAX_LIT_CODES; i++) {
        mla_uint8_t len = __mla_deflate_fixed_tables.lit_length[i];
        if (len != 0) {
            __mla_deflate_fixed_tables.lit_code[i] = next_code[len]++;
        }
    }

    // Distance codes: all 5 bits (codes 0..29)
    for (i = 0; i < MLA_DEFLATE_MAX_DIST_CODES; i++) {
        __mla_deflate_fixed_tables.dist_length[i] = 5;
        __mla_deflate_fixed_tables.dist_code[i] = i;
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
    mla_byte_t window[MLA_DEFLATE_WINDOW_SIZE];
    mla_size_t window_pos;
    mla_size_t window_filled;

    // Hash chain for LZ77 matching
    mla_uint16_t hash_head[MLA_DEFLATE_HASH_SIZE];    // Head of hash chain
    mla_uint16_t hash_prev[MLA_DEFLATE_WINDOW_SIZE];  // Previous entry in hash chain

    // Input buffer for pending data
    mla_byte_t input_buf[MLA_DEFLATE_WINDOW_SIZE];
    mla_size_t input_len;

    mla_bool_t block_started;
    mla_bool_t finished;
};

struct __mla_deflate_compress_state_initializer {

    static __mla_deflate_compress_state_t init() {
        __mla_deflate_compress_state_t result = {
            mla_stream_noop_output(),      // base_output
            {nullptr, {}, 0, 0, 0, false}, // writer
            {}, 0, 0,                      // window, window_pos, window_filled
            {}, {},                        // hash_head, hash_prev
            {}, 0,                         // input_buf, input_len
            false, false                   // block_started, finished
        };
        return result;
    }
};

static mla_uint16_t __mla_deflate_hash3(const mla_byte_t *data) {
    return (mla_uint16_t)(((mla_uint32_t)data[0] | ((mla_uint32_t)data[1] << 8) | ((mla_uint32_t)data[2] << 16)) & MLA_DEFLATE_HASH_MASK);
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
static void __mla_deflate_emit_literal(__mla_deflate_bit_writer_t &writer, mla_byte_t literal) {
    mla_uint16_t code = __mla_deflate_fixed_tables.lit_code[literal];
    mla_uint8_t length = __mla_deflate_fixed_tables.lit_length[literal];
    __mla_deflate_bit_writer_write_reversed(writer, code, length);
}

// Emit a length/distance pair using fixed Huffman coding
static void __mla_deflate_emit_match(__mla_deflate_bit_writer_t &writer, mla_uint16_t match_len, mla_uint16_t match_dist) {
    // Emit length code
    mla_uint16_t len_idx = __mla_deflate_length_code(match_len);
    mla_uint16_t len_symbol = 257 + len_idx;
    mla_uint16_t code = __mla_deflate_fixed_tables.lit_code[len_symbol];
    mla_uint8_t code_len = __mla_deflate_fixed_tables.lit_length[len_symbol];
    __mla_deflate_bit_writer_write_reversed(writer, code, code_len);

    // Emit length extra bits (LSB first, no reversal)
    mla_uint8_t extra = __mla_deflate_length_extra[len_idx];
    if (extra > 0) {
        mla_uint16_t extra_val = match_len - __mla_deflate_length_base[len_idx];
        __mla_deflate_bit_writer_write(writer, extra_val, extra);
    }

    // Emit distance code (fixed: 5 bits, reversed)
    mla_uint16_t dist_idx = __mla_deflate_dist_code(match_dist);
    __mla_deflate_bit_writer_write_reversed(writer, __mla_deflate_fixed_tables.dist_code[dist_idx], 5);

    // Emit distance extra bits
    mla_uint8_t dist_extra = __mla_deflate_dist_extra[dist_idx];
    if (dist_extra > 0) {
        mla_uint16_t dist_extra_val = match_dist - __mla_deflate_dist_base[dist_idx];
        __mla_deflate_bit_writer_write(writer, dist_extra_val, dist_extra);
    }
}

// Emit end-of-block marker
static void __mla_deflate_emit_end_of_block(__mla_deflate_bit_writer_t &writer) {
    mla_uint16_t code = __mla_deflate_fixed_tables.lit_code[MLA_DEFLATE_END_OF_BLOCK];
    mla_uint8_t length = __mla_deflate_fixed_tables.lit_length[MLA_DEFLATE_END_OF_BLOCK];
    __mla_deflate_bit_writer_write_reversed(writer, code, length);
}

// Find the best match in the window using hash chains
static mla_bool_t __mla_deflate_find_match(__mla_deflate_compress_state_t &state,
                                           const mla_byte_t *data, mla_size_t data_len, mla_size_t pos,
                                           mla_uint16_t &best_len, mla_uint16_t &best_dist) {
    best_len = 0;
    best_dist = 0;

    if (data_len - pos < MLA_DEFLATE_MIN_MATCH) return false;
    if (state.window_filled < MLA_DEFLATE_MIN_MATCH) return false;

    mla_uint16_t hash = __mla_deflate_hash3(data + pos);
    mla_uint16_t chain = state.hash_head[hash];
    mla_uint16_t chain_limit = 64; // Limit chain traversal for speed

    while (chain != 0xFFFF && chain_limit > 0) {
        chain_limit--;

        // Calculate the distance from current window position to chain entry
        mla_size_t chain_pos = chain;
        mla_size_t cur_win_pos = state.window_pos;

        mla_size_t dist;
        if (cur_win_pos >= chain_pos) {
            dist = cur_win_pos - chain_pos;
        } else {
            dist = MLA_DEFLATE_WINDOW_SIZE - chain_pos + cur_win_pos;
        }

        if (dist == 0 || dist > state.window_filled || dist > MLA_DEFLATE_WINDOW_SIZE) {
            break;
        }

        // Compare bytes
        mla_uint16_t match_len = 0;
        mla_size_t max_len = data_len - pos;
        if (max_len > MLA_DEFLATE_MAX_MATCH) max_len = MLA_DEFLATE_MAX_MATCH;

        for (mla_size_t j = 0; j < max_len; j++) {
            mla_size_t win_idx = (MLA_DEFLATE_WINDOW_SIZE + state.window_pos - dist + j) & MLA_DEFLATE_WINDOW_MASK;
            if (state.window[win_idx] == data[pos + j]) {
                match_len++;
            } else {
                break;
            }
        }

        if (match_len >= MLA_DEFLATE_MIN_MATCH && match_len > best_len) {
            best_len = match_len;
            best_dist = (mla_uint16_t)dist;
            if (best_len >= MLA_DEFLATE_MAX_MATCH) break;
        }

        chain = state.hash_prev[chain];
    }

    return best_len >= MLA_DEFLATE_MIN_MATCH;
}

// Add bytes to the sliding window and update hash chains
static void __mla_deflate_window_add(__mla_deflate_compress_state_t &state, const mla_byte_t *data, mla_size_t length) {
    for (mla_size_t i = 0; i < length; i++) {
        state.window[state.window_pos] = data[i];

        // Update hash chains if we have at least 3 bytes in the window to hash
        if (state.window_filled >= 2 && i >= 2) {
            mla_uint16_t hash = __mla_deflate_hash3(data + i - 2);
            mla_size_t pos = (state.window_pos - 2) & MLA_DEFLATE_WINDOW_MASK;
            state.hash_prev[pos] = state.hash_head[hash];
            state.hash_head[hash] = (mla_uint16_t)pos;
        } else if (state.window_filled >= 2) {
            // Use window data for the hash
            mla_byte_t hash_bytes[3];
            hash_bytes[0] = state.window[(MLA_DEFLATE_WINDOW_SIZE + state.window_pos - 2) & MLA_DEFLATE_WINDOW_MASK];
            hash_bytes[1] = state.window[(MLA_DEFLATE_WINDOW_SIZE + state.window_pos - 1) & MLA_DEFLATE_WINDOW_MASK];
            hash_bytes[2] = data[i];
            mla_uint16_t hash = __mla_deflate_hash3(hash_bytes);
            mla_size_t pos = (MLA_DEFLATE_WINDOW_SIZE + state.window_pos - 2) & MLA_DEFLATE_WINDOW_MASK;
            state.hash_prev[pos] = state.hash_head[hash];
            state.hash_head[hash] = (mla_uint16_t)pos;
        }

        state.window_pos = (state.window_pos + 1) & MLA_DEFLATE_WINDOW_MASK;
        if (state.window_filled < MLA_DEFLATE_WINDOW_SIZE) {
            state.window_filled++;
        }
    }
}

// Compress a block of data using LZ77 + fixed Huffman
static void __mla_deflate_compress_block(__mla_deflate_compress_state_t &state, const mla_byte_t *data, mla_size_t length, mla_bool_t is_final) {

    if (!state.block_started) {
        // Write block header: BFINAL (1 bit) + BTYPE (2 bits) = fixed Huffman (01)
        __mla_deflate_bit_writer_write(state.writer, is_final ? 1u : 0u, 1);
        __mla_deflate_bit_writer_write(state.writer, MLA_DEFLATE_BLOCK_FIXED, 2);
        state.block_started = true;
    }

    mla_size_t pos = 0;
    while (pos < length) {
        mla_uint16_t best_len = 0;
        mla_uint16_t best_dist = 0;

        if (__mla_deflate_find_match(state, data, length, pos, best_len, best_dist)) {
            __mla_deflate_emit_match(state.writer, best_len, best_dist);
            __mla_deflate_window_add(state, data + pos, best_len);
            pos += best_len;
        } else {
            __mla_deflate_emit_literal(state.writer, data[pos]);
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
    if (output.write == nullptr) {
        return mla_stream_noop_output();
    }

    __mla_deflate_build_fixed_tables();

    __mla_deflate_compress_state_t *state = static_cast<__mla_deflate_compress_state_t *>(mla_malloc(sizeof(__mla_deflate_compress_state_t)));

    if (state == nullptr) {
        return mla_stream_noop_output();
    }

    mla_memset(state, 0, sizeof(__mla_deflate_compress_state_t));
    state->base_output = output;
    __mla_deflate_bit_writer_init(state->writer, state->base_output);
    state->block_started = false;
    state->finished = false;
    state->window_pos = 0;
    state->window_filled = 0;
    state->input_len = 0;

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

    if (!state->block_started) {
        // Write an empty final block
        __mla_deflate_bit_writer_write(state->writer, 1, 1); // BFINAL = 1
        __mla_deflate_bit_writer_write(state->writer, MLA_DEFLATE_BLOCK_FIXED, 2);
    } else {
        // We need to close the current block and start a final one
        // Emit end-of-block for current block
        __mla_deflate_emit_end_of_block(state->writer);
        state->block_started = false;

        // Write empty final block
        __mla_deflate_bit_writer_write(state->writer, 1, 1); // BFINAL = 1
        __mla_deflate_bit_writer_write(state->writer, MLA_DEFLATE_BLOCK_FIXED, 2);
    }

    // End of block
    __mla_deflate_emit_end_of_block(state->writer);
    __mla_deflate_bit_writer_finish(state->writer);

    state->finished = true;

    return !state->writer.error;
}

///////////////////////////////////////////////////////////////////
/// Decompression Input Wrapper
///////////////////////////////////////////////////////////////////

struct __mla_deflate_decompress_state_t {
    mla_stream_input_t base_input;
    __mla_deflate_bit_reader_t reader;

    // Output window (circular buffer)
    mla_byte_t window[MLA_DEFLATE_WINDOW_SIZE];
    mla_size_t window_pos;

    // Decoded output buffer
    mla_byte_t *output_buf;
    mla_size_t output_buf_size;
    mla_size_t output_buf_pos;   // read position
    mla_size_t output_buf_len;   // amount of decoded data in buffer

    // Huffman tables for current block
    __mla_deflate_huffman_t lit_table;
    __mla_deflate_huffman_t dist_table;

    // Block state
    mla_bool_t block_final;
    mla_bool_t block_active;
    mla_uint8_t block_type;
    mla_size_t uncompressed_remaining; // for uncompressed blocks

    mla_bool_t finished;
    mla_bool_t error;
};

struct __mla_deflate_decompress_state_initializer {

    static __mla_deflate_decompress_state_t init() {
        __mla_deflate_decompress_state_t result = {
            mla_stream_noop_input(),       // base_input
            {nullptr, 0, 0, false},        // reader
            {}, 0,                         // window, window_pos
            nullptr, 0, 0, 0,             // output_buf, output_buf_size, output_buf_pos, output_buf_len
            {{}, {}, 0},                   // lit_table
            {{}, {}, 0},                   // dist_table
            false, false, 0, 0,           // block_final, block_active, block_type, uncompressed_remaining
            false, false                   // finished, error
        };
        return result;
    }
};

mla_user_data_id_init(__mla_stream_deflate_decompress_data_name)

// Build fixed Huffman tables for decompression
static void __mla_deflate_build_fixed_decode_tables(__mla_deflate_huffman_t &lit_table, __mla_deflate_huffman_t &dist_table) {
    mla_uint8_t lengths[MLA_DEFLATE_MAX_LIT_CODES];
    mla_uint16_t i;

    // Literal/Length: 0..143 = 8 bits, 144..255 = 9 bits, 256..279 = 7 bits, 280..285 = 8 bits
    for (i = 0; i <= 143; i++) lengths[i] = 8;
    for (i = 144; i <= 255; i++) lengths[i] = 9;
    for (i = 256; i <= 279; i++) lengths[i] = 7;
    for (i = 280; i <= 285; i++) lengths[i] = 8;

    __mla_deflate_huffman_build(lit_table, lengths, MLA_DEFLATE_MAX_LIT_CODES);

    // Distance: all 5 bits
    mla_uint8_t dist_lengths[MLA_DEFLATE_MAX_DIST_CODES];
    for (i = 0; i < MLA_DEFLATE_MAX_DIST_CODES; i++) dist_lengths[i] = 5;
    __mla_deflate_huffman_build(dist_table, dist_lengths, MLA_DEFLATE_MAX_DIST_CODES);
}

// Decode dynamic Huffman tables from the stream
static mla_bool_t __mla_deflate_decode_dynamic_tables(__mla_deflate_decompress_state_t &state) {
    __mla_deflate_bit_reader_t &reader = state.reader;

    mla_uint32_t hlit = __mla_deflate_bit_reader_read(reader, 5) + 257;
    mla_uint32_t hdist = __mla_deflate_bit_reader_read(reader, 5) + 1;
    mla_uint32_t hclen = __mla_deflate_bit_reader_read(reader, 4) + 4;

    if (reader.error) return false;

    if (hlit > MLA_DEFLATE_MAX_LIT_CODES || hdist > MLA_DEFLATE_MAX_DIST_CODES) {
        state.error = true;
        return false;
    }

    // Read code length code lengths
    mla_uint8_t cl_lengths[MLA_DEFLATE_MAX_CL_CODES];
    mla_memset(cl_lengths, 0, sizeof(cl_lengths));

    for (mla_uint32_t i = 0; i < hclen; i++) {
        cl_lengths[__mla_deflate_cl_order[i]] = (mla_uint8_t)__mla_deflate_bit_reader_read(reader, 3);
        if (reader.error) return false;
    }

    // Build the code length Huffman table
    __mla_deflate_huffman_t cl_table;
    __mla_deflate_huffman_init(cl_table);
    if (!__mla_deflate_huffman_build(cl_table, cl_lengths, MLA_DEFLATE_MAX_CL_CODES)) {
        state.error = true;
        return false;
    }

    // Decode literal/length and distance code lengths
    mla_uint8_t lengths[MLA_DEFLATE_MAX_CODES];
    mla_memset(lengths, 0, sizeof(lengths));
    mla_uint32_t total = hlit + hdist;
    mla_uint32_t idx = 0;

    while (idx < total) {
        mla_int32_t symbol = __mla_deflate_huffman_decode(reader, cl_table);
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
            mla_uint32_t repeat = __mla_deflate_bit_reader_read(reader, 2) + 3;
            if (reader.error) return false;
            mla_uint8_t prev = lengths[idx - 1];
            for (mla_uint32_t r = 0; r < repeat && idx < total; r++) {
                lengths[idx++] = prev;
            }
        } else if (symbol == 17) {
            // Repeat 0 for 3..10 times
            mla_uint32_t repeat = __mla_deflate_bit_reader_read(reader, 3) + 3;
            if (reader.error) return false;
            for (mla_uint32_t r = 0; r < repeat && idx < total; r++) {
                lengths[idx++] = 0;
            }
        } else if (symbol == 18) {
            // Repeat 0 for 11..138 times
            mla_uint32_t repeat = __mla_deflate_bit_reader_read(reader, 7) + 11;
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

    while (state.output_buf_len < state.output_buf_size) {
        mla_int32_t symbol = __mla_deflate_huffman_decode(reader, state.lit_table);

        if (symbol < 0) {
            state.error = true;
            return false;
        }

        if (symbol < 256) {
            // Literal byte
            state.output_buf[state.output_buf_len++] = (mla_byte_t)symbol;
            state.window[state.window_pos] = (mla_byte_t)symbol;
            state.window_pos = (state.window_pos + 1) & MLA_DEFLATE_WINDOW_MASK;
        } else if (symbol == MLA_DEFLATE_END_OF_BLOCK) {
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
                match_len += (mla_uint16_t)__mla_deflate_bit_reader_read(reader, extra);
                if (reader.error) {
                    state.error = true;
                    return false;
                }
            }

            // Decode distance
            mla_int32_t dist_symbol = __mla_deflate_huffman_decode(reader, state.dist_table);
            if (dist_symbol < 0 || dist_symbol >= MLA_DEFLATE_MAX_DIST_CODES) {
                state.error = true;
                return false;
            }
            mla_uint16_t match_dist = __mla_deflate_dist_base[dist_symbol];
            mla_uint8_t dist_extra = __mla_deflate_dist_extra[dist_symbol];
            if (dist_extra > 0) {
                match_dist += (mla_uint16_t)__mla_deflate_bit_reader_read(reader, dist_extra);
                if (reader.error) {
                    state.error = true;
                    return false;
                }
            }

            // Copy from window
            for (mla_uint16_t i = 0; i < match_len; i++) {
                mla_size_t src_pos = (MLA_DEFLATE_WINDOW_SIZE + state.window_pos - match_dist) & MLA_DEFLATE_WINDOW_MASK;
                mla_byte_t byte_val = state.window[src_pos];
                state.output_buf[state.output_buf_len++] = byte_val;
                state.window[state.window_pos] = byte_val;
                state.window_pos = (state.window_pos + 1) & MLA_DEFLATE_WINDOW_MASK;

                if (state.output_buf_len >= state.output_buf_size) {
                    // Output buffer full but match not complete - this is OK,
                    // we'll pick it up on the next call. However, for simplicity,
                    // we just let it fill up and stop here.
                    break;
                }
            }
        }
    }

    return true;
}

// Decode uncompressed block data
static mla_bool_t __mla_deflate_decode_uncompressed_block(__mla_deflate_decompress_state_t &state) {
    while (state.uncompressed_remaining > 0 && state.output_buf_len < state.output_buf_size) {
        mla_byte_t byte_val = 0;
        mla_size_t read_count = state.base_input.read(state.base_input, 0, 1, &byte_val);
        if (read_count == 0) {
            state.error = true;
            return false;
        }
        state.output_buf[state.output_buf_len++] = byte_val;
        state.window[state.window_pos] = byte_val;
        state.window_pos = (state.window_pos + 1) & MLA_DEFLATE_WINDOW_MASK;
        state.uncompressed_remaining--;
    }

    if (state.uncompressed_remaining == 0) {
        state.block_active = false;
    }

    return true;
}

// Decode compressed data into the output buffer
static mla_bool_t __mla_deflate_decompress_fill(__mla_deflate_decompress_state_t &state) {
    state.output_buf_pos = 0;
    state.output_buf_len = 0;

    while (state.output_buf_len < state.output_buf_size && !state.finished && !state.error) {

        if (!state.block_active) {
            // Read block header
            mla_uint32_t bfinal = __mla_deflate_bit_reader_read(state.reader, 1);
            mla_uint32_t btype = __mla_deflate_bit_reader_read(state.reader, 2);

            if (state.reader.error) {
                state.error = true;
                return false;
            }

            state.block_final = (bfinal == 1);
            state.block_type = (mla_uint8_t)btype;
            state.block_active = true;

            if (btype == MLA_DEFLATE_BLOCK_UNCOMPRESSED) {
                // Align to byte boundary
                __mla_deflate_bit_reader_align(state.reader);

                // Read LEN and NLEN
                mla_uint32_t len = __mla_deflate_bit_reader_read(state.reader, 16);
                mla_uint32_t nlen = __mla_deflate_bit_reader_read(state.reader, 16);

                if (state.reader.error) {
                    state.error = true;
                    return false;
                }

                if ((len ^ nlen) != 0xFFFF) {
                    state.error = true;
                    return false;
                }

                state.uncompressed_remaining = len;
            } else if (btype == MLA_DEFLATE_BLOCK_FIXED) {
                __mla_deflate_build_fixed_decode_tables(state.lit_table, state.dist_table);
            } else if (btype == MLA_DEFLATE_BLOCK_DYNAMIC) {
                if (!__mla_deflate_decode_dynamic_tables(state)) {
                    return false;
                }
            } else {
                state.error = true;
                return false;
            }
        }

        if (state.block_type == MLA_DEFLATE_BLOCK_UNCOMPRESSED) {
            if (!__mla_deflate_decode_uncompressed_block(state)) {
                return false;
            }
        } else {
            if (!__mla_deflate_decode_huffman_block(state)) {
                return false;
            }
        }

        if (!state.block_active && state.block_final) {
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

    while (total_read < length) {
        // If there's data in the output buffer, copy it
        if (state->output_buf_pos < state->output_buf_len) {
            mla_size_t available = state->output_buf_len - state->output_buf_pos;
            mla_size_t to_copy = length - total_read;
            if (to_copy > available) to_copy = available;
            mla_memcpy(buffer + offset + total_read, state->output_buf + state->output_buf_pos, to_copy);
            state->output_buf_pos += to_copy;
            total_read += to_copy;
        } else {
            // Need more data
            if (state->finished || state->error) {
                break;
            }

            if (!__mla_deflate_decompress_fill(*state)) {
                break;
            }

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

    __mla_deflate_decompress_state_t *state = static_cast<__mla_deflate_decompress_state_t *>(mla_malloc(sizeof(__mla_deflate_decompress_state_t)));

    if (state == nullptr) {
        return mla_stream_noop_input();
    }

    mla_memset(state, 0, sizeof(__mla_deflate_decompress_state_t));
    state->base_input = input;
    __mla_deflate_bit_reader_init(state->reader, state->base_input);
    state->window_pos = 0;
    state->block_active = false;
    state->block_final = false;
    state->finished = false;
    state->error = false;

    // Allocate output buffer
    mla_size_t buf_size = MLA_DEFLATE_COMPRESS_BUF_SIZE;
    state->output_buf = static_cast<mla_byte_t *>(mla_malloc(buf_size));
    if (state->output_buf == nullptr) {
        mla_free(state);
        return mla_stream_noop_input();
    }
    state->output_buf_size = buf_size;
    state->output_buf_pos = 0;
    state->output_buf_len = 0;

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
