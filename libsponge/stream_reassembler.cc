#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

StreamReassembler::StreamReassembler(const std::size_t capacity)
    :  // the comma operator evaluates the operands from left to right, and the
       // value on the right is used for assignment
    capacity_stream{capacity}
    , capacity_window{capacity}
    , _output{ByteStream(this->capacity_stream)}
    , window{std::vector<char>(this->capacity_window)}
    , received{std::vector<bool>(this->capacity_window)} {
    for (auto iterator = this->received.begin(); iterator != this->received.end(); ++iterator)
        *iterator = false;
}

std::size_t StreamReassembler::stream_to_window_index(const std::uint64_t index) const {
    if (this->capacity_window == 0)
        return 0;
    else
        return std::size_t(index % this->capacity_window);
}

std::size_t StreamReassembler::try_push_substring(const std::string &data, const std::uint64_t index) {
    std::uint64_t string_first = index;
    std::uint64_t string_last = string_first + data.length() - 1;
    std::uint64_t window_first = this->index_stream;
    std::uint64_t window_last = window_first + this->capacity_window - 1;

    /*
     * The string must overlap with the window for some of its bytes to be
     * pushed into the window. Overlap is defined as: string's first byte is
     * not after the window's last byte, AND string's last byte is not before
     * the window's first byte.
     */
    if (!(string_first <= window_last && string_last >= window_first))
        return 0;

    /*
     * The overlap spans from the larger of the two first bytes, through the
     * smaller of the two last bytes
     */
    std::uint64_t overlap_first = string_first >= window_first ? string_first : window_first;
    std::uint64_t overlap_last = string_last <= window_last ? string_last : window_last;

    // copy the overlapped bytes from string into the window
    for (std::uint64_t stream_index = overlap_first; stream_index <= overlap_last; ++stream_index) {
        std::size_t string_index = std::size_t(stream_index - string_first);
        std::size_t window_index = this->stream_to_window_index(stream_index);
        this->window[window_index] = data[string_index];
        this->received[window_index] = true;
    }

    return std::size_t(overlap_last - overlap_first + 1);
}

std::size_t StreamReassembler::contiguous_bytes() const {
    // count the number of contiguous bytes in the window
    std::size_t bytes = 0;
    for (std::size_t offset = 0; offset < this->capacity_window; ++offset) {
        std::size_t window_index = this->stream_to_window_index(std::uint64_t(this->index_stream + offset));
        if (this->received[window_index])
            ++bytes;
        else
            break;
    }
    return bytes;
}

std::size_t StreamReassembler::assemble() {
    // check if there are contiguous bytes in the window
    std::size_t contiguous_length = this->contiguous_bytes();
    if (!contiguous_length)
        return 0;

    /*
     * Extract the contiguous bytes as a string. We need to start at the logical
     * first byte of the window, not simply at the physical first byte of the
     * window, and copy each chart into the string. As the copying proceeds, we
     * may wrap around to the physical beginning of the window, so we cannot use
     * iterator to initialize the string.
     */
    std::string data;
    for (std::size_t offset = 0; offset < contiguous_length; ++offset) {
        std::size_t window_index = this->stream_to_window_index(std::uint64_t(this->index_stream + offset));
        data.append(1, this->window[window_index]);
    }

    // write as much contiguous bytes as possible into the output stream
    std::size_t bytes_written = this->_output.write(data);

    // update object state, including the "received" flags and "index_stream"
    for (std::size_t offset = 0; offset < bytes_written; ++offset) {
        std::size_t window_index = this->stream_to_window_index(std::uint64_t(this->index_stream + offset));
        this->received[window_index] = false;
    }
    this->index_stream += bytes_written;

    return bytes_written;
}

void StreamReassembler::push_substring(const std::string &data, const std::size_t index, const bool eof) {
    // try to push substring and assemble
    if (data.length())
        this->try_push_substring(data, index);
    this->assemble();

    // if it's EOF, remember the index of the terminating byte
    if (eof)
        this->index_eof = index + data.length();
    // if the entire stream has been received, terminate the output stream
    if (this->index_eof == this->index_stream + this->contiguous_bytes())
        this->_output.end_input();
}

std::size_t StreamReassembler::unassembled_bytes() const {
    // count the number of bytes that exist in the window
    std::size_t bytes = 0;
    for (std::size_t offset = 0; offset < this->capacity_window; ++offset) {
        std::size_t window_index = this->stream_to_window_index(std::uint64_t(this->index_stream + offset));
        if (this->received[window_index])
            ++bytes;
    }
    return bytes;
}

bool StreamReassembler::empty() const {
    // if any byte exists in the window, return false; otherwise, return true
    for (std::size_t offset = 0; offset < this->capacity_window; ++offset) {
        std::size_t window_index = this->stream_to_window_index(std::uint64_t(this->index_stream + offset));
        if (this->received[window_index])
            return false;
    }
    return true;
}
