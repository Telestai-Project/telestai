// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Telestai Core developers
// Copyright (c) 2017-2021 The Telestai Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <pow_hash.h>
#include <tinyformat.h>

uint32_t nKAWPOWActivationTime = 0;
uint32_t nALT_PROGPOW_ACTIVATION_TIME = 0;

uint256 CBlockHeader::GetHash() const
{
    // AuxPoW blocks use the Scrypt hash of the pure 6-field header.
    if (this->nVersion.IsAuxpow()) {
        return CPureBlockHeader::GetHash();
    }

    // Pre-KAWPOW blocks use X16R / X16RV2.
    if (nTime < nKAWPOWActivationTime) {
        // TODO: Wire X16RV2 activation timestamp per-network once BlockNetwork
        // or a more elegant mechanism is integrated.  For now always use X16RV2.
        return HashX16RV2(reinterpret_cast<const unsigned char*>(&nVersion),
                          reinterpret_cast<const unsigned char*>(&nNonce) + sizeof(nNonce),
                          hashPrevBlock);
    }

    // Post-KAWPOW, pre-alt-ProgPoW blocks use KAWPOW ProgPow.
    if (nTime < nALT_PROGPOW_ACTIVATION_TIME) {
        return KAWPOWHash_OnlyMix(*this);
    }

    // alt-ProgPoW blocks.
    return AltProgPowHash_OnlyMix(*this);
}

uint256 CBlockHeader::GetHashFull(uint256& mix_hash_out) const
{
    if (nTime < nKAWPOWActivationTime) {
        return HashX16RV2(reinterpret_cast<const unsigned char*>(&nVersion),
                          reinterpret_cast<const unsigned char*>(&nNonce) + sizeof(nNonce),
                          hashPrevBlock);
    }

    if (nTime < nALT_PROGPOW_ACTIVATION_TIME) {
        return KAWPOWHash(*this, mix_hash_out);
    }

    return AltProgPowHash(*this, mix_hash_out);
}

uint256 CBlockHeader::GetX16RHash() const
{
    return HashX16R(reinterpret_cast<const unsigned char*>(&nVersion),
                    reinterpret_cast<const unsigned char*>(&nNonce) + sizeof(nNonce),
                    hashPrevBlock);
}

uint256 CBlockHeader::GetX16RV2Hash() const
{
    return HashX16RV2(reinterpret_cast<const unsigned char*>(&nVersion),
                      reinterpret_cast<const unsigned char*>(&nNonce) + sizeof(nNonce),
                      hashPrevBlock);
}

uint256 CBlockHeader::GetKAWPOWHeaderHash() const
{
    CKAWPOWInput input{*this};
    return (HashWriter{} << input).GetHash();
}

uint256 CBlockHeader::GetAltProgPowHeaderHash() const
{
    CAltProgPowInput input{*this};
    return (HashWriter{} << input).GetHash();
}

std::string CBlockHeader::ToString() const
{
    std::stringstream s;
    s << strprintf(
        "CBlockHeader(ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, "
        "nTime=%u, nBits=%08x, nNonce=%u, nNonce64=%u, nHeight=%u)",
        nVersion.GetFullVersion(),
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce, nNonce64, nHeight);
    return s.str();
}

void CBlockHeader::SetAuxpow(std::shared_ptr<CAuxPow> apow)
{
    if (apow) {
        auxpow = std::move(apow);
        nVersion.SetAuxpow(true);
    } else {
        auxpow.reset();
        nVersion.SetAuxpow(false);
    }
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf(
        "CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, "
        "nTime=%u, nBits=%08x, nNonce=%u, nNonce64=%u, vtx=%u, auxpow=%s)\n",
        GetHash().ToString(),
        nVersion.GetFullVersion(),
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce, nNonce64,
        vtx.size(),
        auxpow ? auxpow->ToString() : "null");
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
