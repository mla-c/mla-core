//
// Created by christian on 9/13/25.
//

#include "mla_stream.h"

struct __mla_stream_buffer_manager {
    mla_byte_t* buffer;
    mla_size_t size;
    mla_size_t position;
};


mla_size_t __mla_stream_noop_write(mla_callback_userdata userdata, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void) userdata;
    (void) offset;
    (void) buffer;
    return length;
}

mla_size_t __mla_steam_noop_read(mla_callback_userdata userdata, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    (void) userdata;
    (void) offset;
    (void) buffer;
    (void) length;
    return 0;
}

mla_stream_output_t mla_stream_noop_output() {
    return {
        0,
        __mla_stream_noop_write,
        mla_buffer_reference_noOwner()
    };
}

mla_stream_input_t mla_stream_noop_input() {
    return {
        0,
        __mla_steam_noop_read,
        mla_buffer_reference_noOwner()
    };
}

mla_size_t __mla_stream_input_read_from_buffer(mla_callback_userdata userdata, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    __mla_stream_buffer_manager* manager = reinterpret_cast<__mla_stream_buffer_manager*>(userdata);

    if (manager->position + length > manager->size) {
        length = manager->size - manager->position;
    }

    if (length == 0) {
        return 0;
    }

    mla_memcpy(buffer + offset, static_cast<mla_byte_t*>(manager->buffer) + manager->position, length);
    manager->position += length;

    return length;
}

mla_stream_input_t mla_stream_input_from_buffer(mla_byte_t* buffer, mla_size_t size) {

    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_malloc(sizeof(__mla_stream_buffer_manager)));
    bufferManager->buffer = buffer;
    bufferManager->size = size;
    bufferManager->position = 0;

    return {
        reinterpret_cast<mla_callback_userdata>(bufferManager),
        __mla_stream_input_read_from_buffer,
        mla_buffer_reference(bufferManager)
    };
}

mla_size_t __mla_stream_output_write_to_buffer(mla_callback_userdata userdata, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    __mla_stream_buffer_manager* manager = reinterpret_cast<__mla_stream_buffer_manager*>(userdata);

    if (manager->position + length > manager->size) {
        length = manager->size - manager->position;
    }

    if (length == 0) {
        return 0;
    }

    mla_memcpy(static_cast<mla_byte_t*>(manager->buffer) + manager->position, buffer + offset, length);
    manager->position += length;

    return length;
}

mla_stream_output_t mla_stream_output_to_buffer(mla_byte_t* buffer, mla_size_t size) {

    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_malloc(sizeof(__mla_stream_buffer_manager)));
    bufferManager->buffer = buffer;
    bufferManager->size = size;
    bufferManager->position = 0;

    return {
        reinterpret_cast<mla_callback_userdata>(bufferManager),
        __mla_stream_output_write_to_buffer,
        mla_buffer_reference(bufferManager)
    };
}

mla_stream_input_t mla_stream_input_from_buffer(mla_size_t size) {

    // Allocate the buffer manager and the buffer in one allocation
    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_malloc(sizeof(__mla_stream_buffer_manager) + size));
    bufferManager->buffer = ((mla_byte_t*)bufferManager) + size;
    bufferManager->size = size;
    bufferManager->position = 0;

    return {
        reinterpret_cast<mla_callback_userdata>(bufferManager),
        __mla_stream_input_read_from_buffer,
        mla_buffer_reference(bufferManager)
    };

}

mla_stream_output_t mla_stream_output_to_buffer(mla_size_t size) {

    __mla_stream_buffer_manager* bufferManager = static_cast<__mla_stream_buffer_manager*>(mla_malloc(sizeof(__mla_stream_buffer_manager)));
    bufferManager->buffer = ((mla_byte_t*)bufferManager) + size;
    bufferManager->size = size;
    bufferManager->position = 0;

    return {
        reinterpret_cast<mla_callback_userdata>(bufferManager),
        __mla_stream_output_write_to_buffer,
        mla_buffer_reference(bufferManager)
    };

}

mla_size_t __mla_stream_std_input_read(mla_callback_userdata userdata, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    (void)userdata;
    mla_char_t tempBuffer[256] = {0};

    mla_size_t char_read = mla_std_read(tempBuffer, sizeof(tempBuffer));

    if (char_read == 0) {
        return 0; // No data read
    }

    // Copy to the provided buffer
    mla_size_t toCopy = mla_min(char_read, length);
    mla_memcpy(buffer + offset, tempBuffer, toCopy);
    return toCopy;
}


mla_stream_input_t mla_stream_input_stdin() {

    return {
        0,
        __mla_stream_std_input_read,
        mla_buffer_reference_noOwner()
    };

}

mla_size_t __mla_stream_std_output_write(mla_callback_userdata userdata, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    (void)userdata;
    mla_char_t* tempBuffer = reinterpret_cast<mla_char_t*>(mla_malloc(sizeof(mla_char_t) * (length + 1)));

    if (tempBuffer == nullptr) {
        return 0; // Memory allocation failed
    }

    mla_memcpy(tempBuffer, buffer + offset, length);
    tempBuffer[length] = '\0'; // Null-terminate the string
    mla_printf("%s", tempBuffer);

    mla_free(tempBuffer);

    return length;

}

mla_stream_output_t mla_stream_output_stdout() {

    return {
        0,
        __mla_stream_std_output_write,
        mla_buffer_reference_noOwner()
    };

}
