#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

/**
 * @brief Transform a 64-bit ASN into a WrappingInt32 SN.
 * 
 * @param asn The ASN
 * @param isn The ISN
 * @return WrappingInt32 
 */
WrappingInt32 wrap(uint64_t asn, WrappingInt32 isn) {
    // the ASN is the distance between the ISN and its 64-bit RSN
    uint64_t rsn = asn + uint64_t(isn.raw_value());
    
    // we wrap the 64-bit RSN by taking only its lower 32 bits
    uint64_t mask_l32 = (1ul << 32) - 1;
    uint32_t value = uint32_t(rsn & mask_l32);

    return WrappingInt32{value};
}

/**
 * @brief Transform a WrappingInt32 SN into a 64-bit ASN.
 * 
 * @note This function makes the assumption that the ASN is no smaller than the
 * checkpoint.
 * 
 * @param sn The SN
 * @param isn The ISN
 * @param checkpoint A 64-bit number close to ASN
 * @return uint64_t 
 */
uint64_t unwrap(WrappingInt32 sn, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t mask_l32 = (1ul << 32) - 1;
    uint64_t mask_u32 = ~mask_l32;
    
    // lower 32 bits of ASN is obtained by subtracting ISN from SN
    uint32_t asn_l32 = sn.raw_value() - isn.raw_value();
    uint32_t checkpoint_l32 = uint32_t(checkpoint & mask_l32);
    uint64_t checkpoint_u32 = checkpoint & mask_u32;
    
    // the upper 32 bits of the checkpoint and the upper 32 bits of the ASN
    // differ by at most one bit
    uint64_t asn_left = checkpoint_u32 - (1ul << 32) + uint64_t(asn_l32);
    uint64_t asn_mid = checkpoint_u32 + uint64_t(asn_l32);
    uint64_t asn_right = checkpoint_u32 + (1ul << 32) + uint64_t(asn_l32);
    
    uint64_t diff_left = checkpoint - asn_left;
    uint64_t diff_right = asn_right - checkpoint;
    uint64_t diff_mid = checkpoint_l32 >= asn_l32 ? checkpoint - asn_mid : asn_mid - checkpoint;

    uint64_t diff_min = diff_mid;
    // if the upper 32 bits of the checkpoint is all-zero, then there's no ASN to the left
    if (checkpoint_u32 > mask_l32)
        diff_min = diff_min <= diff_left ? diff_min : diff_left;
    // if the upper 32 bits of the checkpoint is all-one, then there's no ASN to the right
    if (checkpoint_u32 < mask_u32)
        diff_min = diff_min <= diff_right ? diff_min : diff_right;

    // the ASN closest to the checkpoint is the true ASN, and always default to
    // the middle ASN and preserve the upper 32 bits of the checkpoint if there are ties
    if (diff_min == diff_mid)
        return asn_mid;
    else if (diff_min == diff_left)
        return asn_left;
    else
        return asn_right;
}
