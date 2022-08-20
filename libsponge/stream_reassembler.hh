#ifndef SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#define SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH

#include "byte_stream.hh"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief A class that assembles byte sequences into an in-order byte stream.
 */
class StreamReassembler {
  private:
    std::size_t capacity_stream;  // The max number of bytes in the output stream
    std::size_t capacity_window;  // The max number of bytes in the buffer window

    ByteStream _output;          // The output stream
    std::vector<char> window;    // The buffer window
    std::vector<bool> received;  // Flags noting which window bytes are received

    std::uint64_t index_stream{0};               // The stream index of the first byte in the window
    std::uint64_t index_eof{~std::uint64_t(0)};  // The stream index of the eof, one past the last byte

    /**
     * @brief Converts a stream index into a window index.
     *
     * @param index The stream index.
     * @return std::uint64_t The window index.
     */
    std::size_t stream_to_window_index(const std::uint64_t index) const;

    /**
     * @brief Try to push the string `data`, which starts at `index` in the
     * stream, into the window.
     *
     * @param data A string.
     * @param index Where `data` starts in the stream.
     *
     * @return std::size_t The number of contiguous bytes (can start from anywhere
     * in `data`) that was successfully pushed into the window.
     */
    std::size_t try_push_substring(const std::string &data, const std::uint64_t index);

    /**
     * @brief Returns the number of contiguous bytes in the window, starting from
     * the beginning of the window.
     *
     * @return std::size_t The number of contiguous bytes.
     */
    std::size_t contiguous_bytes() const;

    /**
     * @brief Assemble the unassenbled bytes in the window, and write as many as
     * possible into the output stream.
     *
     * @return std::size_t The number of bytes written into the output stream.
     */
    std::size_t assemble();

  public:
    /**
     * @brief Construct a new Stream Reassembler object.
     * @note Assembled bytes are written into the output stream, and unassembled
     * bytes are kept in the buffer window.
     *
     * @param capacity The total number of assembled + unassembled bytes that
     * can be stored in the object.
     */
    StreamReassembler(const std::size_t capacity);

    /**
     * @brief Push the string `data`, which starts at `index` in stream, into
     * the window, then assemble and write any contiguous bytes into the output
     * stream.
     *
     * @param data A string.
     * @param index Where `data` starts in the stream.
     * @param eof Whether the last byte of `data` is the last byte of the stream.
     */
    void push_substring(const std::string &data, const std::uint64_t index, const bool eof);

    /**
     * @brief Return the output stream.
     *
     * @return ByteStream The output stream.
     */
    //!@{
    const ByteStream &stream_out() const { return _output; }
    ByteStream &stream_out() { return _output; }
    //!@}

    /**
     * @brief Returns the number of unassembled bytes, i.e. the number of bytes
     * in the window.
     * @note If a byte in the stream has been pushed more than once, it should
     * only be counted once for the purpose of this function.
     *
     * @return std::size_t The number of bytes in the window.
     */
    std::size_t unassembled_bytes() const;

    /**
     * @brief Returns whether the window is empty, i.e. whether there's any
     * unassembled bytes.
     *
     * @return true If there's no unassembled bytes.
     * @return false If there's unassembled bytes.
     */
    bool empty() const;
};

#endif  // SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
