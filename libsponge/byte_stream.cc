#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

ByteStream::ByteStream(const size_t bytes): buffer{std::vector<char>(bytes)}, capacity{bytes} {}

size_t ByteStream::write(const std::string &data) {
    // if the stream has ended or the buffer is full, no more bytes can be written
    if (this->input_ended() || this->remaining_capacity() == 0)
        return 0;

    size_t written = 0;
    for (auto source = data.begin(); source != data.end() && this->remaining_capacity();
         ++source, ++this->size, ++written) {
        size_t destination = this->wrap_index(this->start + this->buffer_size());
        this->buffer[destination] = *source;
    }
    this->_bytes_written += written;
    return written;
}

std::string ByteStream::peek_output(const size_t len) const {
    std::string data;

    // if the buffer is empty, no more bytes can be read
    if (this->buffer_empty())
        return data;

    for (size_t _bytes_peeked = 0, remaining_size = this->buffer_size(); remaining_size && _bytes_peeked < len;
         --remaining_size, ++_bytes_peeked) {
        size_t index = this->wrap_index(this->start + _bytes_peeked);
        data.append(1, this->buffer[index]);
    }
    return data;
}

void ByteStream::pop_output(const size_t len) {
    this->_bytes_read += len <= this->buffer_size() ? len : this->buffer_size();
    this->size = len <= this->buffer_size() ? this->size - len : 0;
    this->start = this->wrap_index(this->start + len);
}

std::string ByteStream::read(const size_t len) {
    std::string data = this->peek_output(len);
    this->pop_output(len);
    return data;
}

size_t ByteStream::buffer_size() const { return this->size; }

bool ByteStream::buffer_empty() const { return this->buffer_size() == 0; }

bool ByteStream::eof() const { return this->buffer_empty() && this->input_ended(); }

size_t ByteStream::remaining_capacity() const { return this->capacity - this->buffer_size(); }
