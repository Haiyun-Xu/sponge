#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

/**
 * @brief Process a received TCP segment.
 *
 * @param seg The received TCP segment
 */
void TCPReceiver::segment_received(const TCPSegment &seg) {
    // if the segment contains SYN flag, then the SN field is the ISN
    if (seg.header().syn) {
        this->ISN = seg.header().seqno.raw_value();
        /*
         * only initialize ASN if it hasn't been initialized, so as to be
         * idempotent if the SYN segment was received multiple times with
         * delays in between.
         */
        if (this->ASN == 0)
            this->ASN = 1;
    }

    // only perform the following if a segment with SYN flag was received
    if (this->ASN != 0) {
        /*
         * the stream index does not account for SYN or FIN. If the segment has the
         * SYN flag, its SN field equals to the ISN, so the SN-derived ASN will be 0;
         * the correct stream index in this case is also 0. If the segment doesn't
         * have the SYN flag, the SN-derived ASN has accounted for SYN flag as its
         * 0th byte, so ASN is one index larger than the stream index; the correct
         * stream index in this case is ASN - 1.
         */
        uint64_t stream_index =
            seg.header().syn ? 0 : unwrap(seg.header().seqno, WrappingInt32(this->ISN), this->ASN) - 1;
        string data = seg.payload().copy();

        size_t size_before = this->stream_out().buffer_size();
        this->_reassembler.push_substring(data, stream_index, seg.header().fin);
        size_t size_after = this->stream_out().buffer_size();

        // if any bytes were written into the stream, then they were contiguous,
        // so the AAN should be forwarded accordingly.
        size_t size_diff = size_after - size_before;
        this->ASN += size_diff;

        // if the input has ended, increment ASN to account for FIN
        if (this->stream_out().input_ended())
            this->ASN++;
    }
}

/**
 * @brief Returns the 32-bit acknowledgement number, or an empty std::optional
 * object if the ISN hasn't been received.
 *
 * @return optional<WrappingInt32>
 */
optional<WrappingInt32> TCPReceiver::ackno() const {
    /*
     * If the ISN hasn't been received, then the ASN is zero. Otherwise, the ASN
     * is also the AAN, and the AN can be constructed from the AAN and ISN.
     */
    if (this->ASN == 0)
        return {};
    else
        return wrap(this->ASN, WrappingInt32(this->ISN));
}

/**
 * @brief Returns the size of the receiving window, i.e. the distance from
 * the first receivable byte to the first unreceivable byte.
 *
 * @return size_t
 */
size_t TCPReceiver::window_size() const { return this->_capacity - this->stream_out().buffer_size(); }
