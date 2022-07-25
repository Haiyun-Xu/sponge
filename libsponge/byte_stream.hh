#ifndef SPONGE_LIBSPONGE_BYTE_STREAM_HH
#define SPONGE_LIBSPONGE_BYTE_STREAM_HH

#include <string>
#include <vector>

//! \brief An in-order byte stream.

//! Bytes are written on the "input" side and read from the "output"
//! side.  The byte stream is finite: the writer can end the input,
//! and then no more bytes can be written.
class ByteStream {
  private:
    // Your code here -- add private members as necessary.

    // Hint: This doesn't need to be a sophisticated data structure at
    // all, but if any of your tests are taking longer than a second,
    // that's a sign that you probably want to keep exploring
    // different approaches.

    bool _error{};        //!< Flag indicating that the stream suffered an error.
    bool stream_ended{};  //!< Flag indicating that the stream has ended.

    std::vector<char> buffer;
    size_t capacity;
    size_t size = 0;   //!< Size of the in-transit sequence.
    size_t start = 0;  //!< Index of the first byte of the in-transit sequence.

    size_t _bytes_written = 0;
    size_t _bytes_read = 0;

    size_t wrap_index(const size_t index) const { return this->capacity == 0 ? 0 : index % this->capacity; }

  public:
    //! Construct a stream with room for `capacity` bytes.
    ByteStream(const size_t capacity);

    //! \name "Input" interface for the writer
    //!@{

    //! Write a string of bytes into the stream. Write as many
    //! as will fit, and return how many were written.
    //! \returns the number of bytes accepted into the stream
    size_t write(const std::string &data);

    //! \returns the number of additional bytes that the stream has space for
    size_t remaining_capacity() const;

    //! Signal that the byte stream has reached its ending
    void end_input() { this->stream_ended = true; }

    //! Indicate that the stream suffered an error.
    void set_error() { this->_error = true; }
    //!@}

    //! \name "Output" interface for the reader
    //!@{

    //! Peek at next "len" bytes of the stream
    //! \returns a string
    std::string peek_output(const size_t len) const;

    //! Remove bytes from the buffer
    void pop_output(const size_t len);

    //! Read (i.e., copy and then pop) the next "len" bytes of the stream
    //! \returns a string
    std::string read(const size_t len);

    //! \returns `true` if the stream input has ended
    bool input_ended() const { return this->stream_ended; }

    //! \returns `true` if the stream has suffered an error
    bool error() const { return this->_error; }

    //! \returns the maximum amount that can currently be read from the stream
    size_t buffer_size() const;

    //! \returns `true` if the buffer is empty
    bool buffer_empty() const;

    //! \returns `true` if the output has reached the ending
    bool eof() const;
    //!@}

    //! \name General accounting
    //!@{

    //! Total number of bytes written
    size_t bytes_written() const { return this->_bytes_written; }

    //! Total number of bytes popped
    size_t bytes_read() const { return this->_bytes_read; }
    //!@}
};

#endif  // SPONGE_LIBSPONGE_BYTE_STREAM_HH
