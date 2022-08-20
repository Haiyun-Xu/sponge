#ifndef SPONGE_LIBSPONGE_TCP_RECEIVER_HH
#define SPONGE_LIBSPONGE_TCP_RECEIVER_HH

#include "byte_stream.hh"
#include "stream_reassembler.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <optional>

//! \brief The "receiver" part of a TCP implementation.

//! Receives and reassembles segments into a ByteStream, and computes
//! the acknowledgment number and window size to advertise back to the
//! remote TCPSender.
class TCPReceiver {
    //! Our data structure for re-assembling bytes.
    StreamReassembler _reassembler;

    size_t _capacity;  //! The maximum number of bytes we'll store.
    uint32_t ISN{0};   //! The initial sequence number.
    uint64_t ASN{0};   //! The absolute sequence number, also the absolute acknowledgement number.

  public:
    /**
     * @brief Construct a new TCPReceiver object.
     *
     * @param capacity The maximum number of bytes that the receiver will store
     * in its StreamReassembler buffer.
     */
    TCPReceiver(const size_t capacity) : _reassembler(capacity), _capacity(capacity) {}

    //! \brief number of bytes stored but not yet reassembled
    size_t unassembled_bytes() const { return _reassembler.unassembled_bytes(); }

    //! \name "Output" interface for the reader
    //!@{
    ByteStream &stream_out() { return _reassembler.stream_out(); }
    const ByteStream &stream_out() const { return _reassembler.stream_out(); }
    //!@}

    /**
     * @brief Process a received TCP segment.
     *
     * @param seg The received TCP segment
     */
    void segment_received(const TCPSegment &seg);

    //! \name Accessors to provide feedback to the remote TCPSender
    //!@{

    /**
     * @brief Returns the 32-bit acknowledgement number, or an empty std::optional
     * object if the ISN hasn't been received.
     *
     * @return std::optional<WrappingInt32>
     */
    std::optional<WrappingInt32> ackno() const;

    /**
     * @brief Returns the size of the receiving window, i.e. the distance from
     * the first receivable byte to the first unreceivable byte.
     *
     * @return size_t
     */
    size_t window_size() const;
    //!@}
};

#endif  // SPONGE_LIBSPONGE_TCP_RECEIVER_HH
