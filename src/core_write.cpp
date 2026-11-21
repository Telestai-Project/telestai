// Copyright (c) 2009-2022 The Telestai Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <core_io.h>

#include <addresstype.h>
#include <assets/assettypes.h>
#include <base58.h>
#include <common/system.h>
#include <consensus/amount.h>
#include <consensus/consensus.h>
#include <consensus/validation.h>
#include <key_io.h>
#include <script/descriptor.h>
#include <script/script.h>
#include <script/solver.h>
#include <serialize.h>
#include <span.h>
#include <streams.h>
#include <undo.h>
#include <univalue.h>
#include <util/check.h>
#include <util/strencodings.h>

#include <map>
#include <string>
#include <vector>

UniValue ValueFromAmount(const CAmount amount)
{
    static_assert(COIN > 1);
    int64_t quotient = amount / COIN;
    int64_t remainder = amount % COIN;
    if (amount < 0) {
        quotient = -quotient;
        remainder = -remainder;
    }
    return UniValue(UniValue::VNUM,
            strprintf("%s%d.%08d", amount < 0 ? "-" : "", quotient, remainder));
}

std::string FormatScript(const CScript& script)
{
    std::string ret;
    CScript::const_iterator it = script.begin();
    opcodetype op;
    while (it != script.end()) {
        CScript::const_iterator it2 = it;
        std::vector<unsigned char> vch;
        if (script.GetOp(it, op, vch)) {
            if (op == OP_0) {
                ret += "0 ";
                continue;
            } else if ((op >= OP_1 && op <= OP_16) || op == OP_1NEGATE) {
                ret += strprintf("%i ", op - OP_1NEGATE - 1);
                continue;
            } else if (op >= OP_NOP && op <= OP_NOP10) {
                std::string str(GetOpName(op));
                if (str.substr(0, 3) == std::string("OP_")) {
                    ret += str.substr(3, std::string::npos) + " ";
                    continue;
                }
            }
            if (vch.size() > 0) {
                ret += strprintf("0x%x 0x%x ", HexStr(std::vector<uint8_t>(it2, it - vch.size())),
                                               HexStr(std::vector<uint8_t>(it - vch.size(), it)));
            } else {
                ret += strprintf("0x%x ", HexStr(std::vector<uint8_t>(it2, it)));
            }
            continue;
        }
        ret += strprintf("0x%x ", HexStr(std::vector<uint8_t>(it2, script.end())));
        break;
    }
    return ret.substr(0, ret.empty() ? ret.npos : ret.size() - 1);
}

const std::map<unsigned char, std::string> mapSigHashTypes = {
    {static_cast<unsigned char>(SIGHASH_ALL), std::string("ALL")},
    {static_cast<unsigned char>(SIGHASH_ALL|SIGHASH_ANYONECANPAY), std::string("ALL|ANYONECANPAY")},
    {static_cast<unsigned char>(SIGHASH_NONE), std::string("NONE")},
    {static_cast<unsigned char>(SIGHASH_NONE|SIGHASH_ANYONECANPAY), std::string("NONE|ANYONECANPAY")},
    {static_cast<unsigned char>(SIGHASH_SINGLE), std::string("SINGLE")},
    {static_cast<unsigned char>(SIGHASH_SINGLE|SIGHASH_ANYONECANPAY), std::string("SINGLE|ANYONECANPAY")},
};

std::string SighashToStr(unsigned char sighash_type)
{
    const auto& it = mapSigHashTypes.find(sighash_type);
    if (it == mapSigHashTypes.end()) return "";
    return it->second;
}

/**
 * Create the assembly string representation of a CScript object.
 * @param[in] script    CScript object to convert into the asm string representation.
 * @param[in] fAttemptSighashDecode    Whether to attempt to decode sighash types on data within the script that matches the format
 *                                     of a signature. Only pass true for scripts you believe could contain signatures. For example,
 *                                     pass false, or omit the this argument (defaults to false), for scriptPubKeys.
 */
std::string ScriptToAsmStr(const CScript& script, const bool fAttemptSighashDecode)
{
    std::string str;
    opcodetype opcode;
    std::vector<unsigned char> vch;
    CScript::const_iterator pc = script.begin();
    while (pc < script.end()) {
        if (!str.empty()) {
            str += " ";
        }
        if (!script.GetOp(pc, opcode, vch)) {
            str += "[error]";
            return str;
        }
        if (0 <= opcode && opcode <= OP_PUSHDATA4) {
            if (vch.size() <= static_cast<std::vector<unsigned char>::size_type>(4)) {
                str += strprintf("%d", CScriptNum(vch, false).getint());
            } else {
                // the IsUnspendable check makes sure not to try to decode OP_RETURN data that may match the format of a signature
                if (fAttemptSighashDecode && !script.IsUnspendable()) {
                    std::string strSigHashDecode;
                    // goal: only attempt to decode a defined sighash type from data that looks like a signature within a scriptSig.
                    // this won't decode correctly formatted public keys in Pubkey or Multisig scripts due to
                    // the restrictions on the pubkey formats (see IsCompressedOrUncompressedPubKey) being incongruous with the
                    // checks in CheckSignatureEncoding.
                    if (CheckSignatureEncoding(vch, SCRIPT_VERIFY_STRICTENC, nullptr)) {
                        const unsigned char chSigHashType = vch.back();
                        const auto it = mapSigHashTypes.find(chSigHashType);
                        if (it != mapSigHashTypes.end()) {
                            strSigHashDecode = "[" + it->second + "]";
                            vch.pop_back(); // remove the sighash type byte. it will be replaced by the decode.
                        }
                    }
                    str += HexStr(vch) + strSigHashDecode;
                } else {
                    str += HexStr(vch);
                }
            }
        } else {
            str += GetOpName(opcode);
        }
    }
    return str;
}

std::string EncodeHexTx(const CTransaction& tx)
{
    DataStream ssTx;
    ssTx << TX_WITH_WITNESS(tx);
    return HexStr(ssTx);
}

namespace {
/**
 * Serialized asset data: the push that follows OP_TLS_ASSET, minus its four marker bytes.
 * Same extraction as ExtractMewcAssetSerializedPayload() in assets/assets.cpp, which this
 * library cannot link against.
 */
bool GetAssetPayload(const CScript& script, std::vector<unsigned char>& payload)
{
    CScript::const_iterator pc = script.begin();
    opcodetype opcode;
    std::vector<unsigned char> vch;
    while (pc < script.end()) {
        if (!script.GetOp(pc, opcode, vch)) return false;
        if (opcode != OP_TLS_ASSET) continue;
        if (!script.GetOp(pc, opcode, vch) || vch.size() < 4) return false;
        payload.assign(vch.begin() + 4, vch.end());
        return true;
    }
    return false;
}

template <typename T>
bool DecodeAssetPayload(const std::vector<unsigned char>& payload, T& obj)
{
    try {
        DataStream ss{payload};
        ss >> obj;
    } catch (const std::exception&) {
        return false;
    }
    return true;
}

/** Asset payloads reference either an IPFS hash (34 bytes, shown as base58) or a txid (32 bytes, shown as hex). */
std::string EncodeAssetHash(const std::string& raw)
{
    if (raw.size() == 34) return EncodeBase58(MakeUCharSpan(raw));
    if (raw.size() == 32) return HexStr(MakeUCharSpan(raw));
    return {};
}

/** "asset" object for new, reissue and transfer asset outputs (same fields as Telestai 2.1.x). */
UniValue AssetToUniv(const CScript& script, const TxoutType type)
{
    UniValue info{UniValue::VOBJ};

    int asset_type{0};
    bool is_owner{false};
    std::vector<unsigned char> payload;
    if (!script.IsAssetScript(asset_type, is_owner) || !GetAssetPayload(script, payload)) return info;

    if (type == TxoutType::TRANSFER_ASSET) {
        CAssetTransfer transfer;
        if (!DecodeAssetPayload(payload, transfer)) return info;
        info.pushKV("name", transfer.strName);
        info.pushKV("amount", ValueFromAmount(transfer.nAmount));
        if (!transfer.message.empty()) info.pushKV("message", EncodeAssetHash(transfer.message));
        if (transfer.nExpireTime != 0) info.pushKV("expire_time", transfer.nExpireTime);
    } else if (type == TxoutType::REISSUE_ASSET) {
        CReissueAsset reissue;
        if (!DecodeAssetPayload(payload, reissue)) return info;
        info.pushKV("name", reissue.strName);
        info.pushKV("amount", ValueFromAmount(reissue.nAmount));
        if (reissue.nUnits >= 0) info.pushKV("units", int{reissue.nUnits});
        info.pushKV("reissuable", reissue.nReissuable > 0);
        if (!reissue.strIPFSHash.empty()) info.pushKV("ipfs_hash", EncodeAssetHash(reissue.strIPFSHash));
    } else if (is_owner) {
        // Owner token (NAME!): the payload is just the name and there is always exactly one.
        std::string name;
        if (!DecodeAssetPayload(payload, name)) return info;
        info.pushKV("name", name);
        info.pushKV("amount", ValueFromAmount(COIN));
    } else {
        CNewAsset asset;
        if (!DecodeAssetPayload(payload, asset)) return info;
        info.pushKV("name", asset.strName);
        info.pushKV("amount", ValueFromAmount(asset.nAmount));
        info.pushKV("units", int{asset.units});
        info.pushKV("reissuable", asset.nReissuable > 0);
        if (asset.nHasIPFS > 0 && !asset.strIPFSHash.empty()) info.pushKV("ipfs_hash", EncodeAssetHash(asset.strIPFSHash));
    }
    return info;
}

/** "asset_data" object for qualifier tags, restrictions and verifier strings (same fields as Telestai 2.1.x). */
UniValue AssetNullDataToUniv(const CScript& script)
{
    UniValue info{UniValue::VOBJ};

    if (script.IsNullAssetTxDataScript()) {
        // OP_TLS_ASSET 0x14 <key hash> <push: name, flag>
        CNullAssetTxData data;
        if (!DecodeAssetPayload(std::vector<unsigned char>(script.begin() + 23, script.end()), data) || data.asset_name.empty()) return info;
        CTxDestination dest;
        ExtractDestination(script, dest);
        if (data.asset_name.front() == '#') {
            info.pushKV("asset_name", data.asset_name);
            info.pushKV("qualifier_type", data.flag ? "adding qualifier" : "removing qualifier");
            info.pushKV("address", EncodeDestination(dest));
        } else if (data.asset_name.front() == '$') {
            info.pushKV("asset_name", data.asset_name);
            info.pushKV("restricted_type", data.flag ? "freezing address" : "unfreezing address");
            info.pushKV("address", EncodeDestination(dest));
        }
    } else if (script.IsNullGlobalRestrictionAssetTxDataScript()) {
        // OP_TLS_ASSET OP_RESERVED OP_RESERVED <push: name, flag>
        CNullAssetTxData data;
        if (!DecodeAssetPayload(std::vector<unsigned char>(script.begin() + 4, script.end()), data)) return info;
        info.pushKV("restricted_name", data.asset_name);
        info.pushKV("restricted_type", data.flag ? "freezing" : "unfreezing");
        info.pushKV("address", "all addresses");
    } else if (script.IsNullAssetVerifierTxDataScript()) {
        // OP_TLS_ASSET OP_RESERVED <push: verifier string>
        CNullAssetTxVerifierString verifier;
        if (!DecodeAssetPayload(std::vector<unsigned char>(script.begin() + 3, script.end()), verifier)) return info;
        info.pushKV("verifier_string", verifier.verifier_string);
    }
    return info;
}
} // namespace

void ScriptToUniv(const CScript& script, UniValue& out, bool include_hex, bool include_address, const SigningProvider* provider)
{
    CTxDestination address;

    out.pushKV("asm", ScriptToAsmStr(script));
    if (include_address) {
        out.pushKV("desc", InferDescriptor(script, provider ? *provider : DUMMY_SIGNING_PROVIDER)->ToString());
    }
    if (include_hex) {
        out.pushKV("hex", HexStr(script));
    }

    std::vector<std::vector<unsigned char>> solns;
    const TxoutType type{Solver(script, solns)};

    if (include_address && ExtractDestination(script, address) && type != TxoutType::PUBKEY) {
        out.pushKV("address", EncodeDestination(address));
    }
    out.pushKV("type", GetTxnOutputType(type));

    // Telestai asset details, for scripts being described as an output.
    if (include_address) {
        switch (type) {
        case TxoutType::NEW_ASSET:
        case TxoutType::REISSUE_ASSET:
        case TxoutType::TRANSFER_ASSET:
            out.pushKV("asset", AssetToUniv(script, type));
            break;
        case TxoutType::RESTRICTED_ASSET_DATA:
            out.pushKV("asset_data", AssetNullDataToUniv(script));
            break;
        default:
            break;
        }
    }
}

void TxToUniv(const CTransaction& tx, const uint256& block_hash, UniValue& entry, bool include_hex, const CTxUndo* txundo, TxVerbosity verbosity)
{
    CHECK_NONFATAL(verbosity >= TxVerbosity::SHOW_DETAILS);

    entry.pushKV("txid", tx.GetHash().GetHex());
    entry.pushKV("hash", tx.GetWitnessHash().GetHex());
    entry.pushKV("version", tx.version);
    entry.pushKV("size", tx.GetTotalSize());
    entry.pushKV("vsize", (GetTransactionWeight(tx) + WITNESS_SCALE_FACTOR - 1) / WITNESS_SCALE_FACTOR);
    entry.pushKV("weight", GetTransactionWeight(tx));
    entry.pushKV("locktime", (int64_t)tx.nLockTime);

    UniValue vin{UniValue::VARR};
    vin.reserve(tx.vin.size());

    // If available, use Undo data to calculate the fee. Note that txundo == nullptr
    // for coinbase transactions and for transactions where undo data is unavailable.
    const bool have_undo = txundo != nullptr;
    CAmount amt_total_in = 0;
    CAmount amt_total_out = 0;

    for (unsigned int i = 0; i < tx.vin.size(); i++) {
        const CTxIn& txin = tx.vin[i];
        UniValue in(UniValue::VOBJ);
        if (tx.IsCoinBase()) {
            in.pushKV("coinbase", HexStr(txin.scriptSig));
        } else {
            in.pushKV("txid", txin.prevout.hash.GetHex());
            in.pushKV("vout", (int64_t)txin.prevout.n);
            UniValue o(UniValue::VOBJ);
            o.pushKV("asm", ScriptToAsmStr(txin.scriptSig, true));
            o.pushKV("hex", HexStr(txin.scriptSig));
            in.pushKV("scriptSig", std::move(o));
        }
        if (!tx.vin[i].scriptWitness.IsNull()) {
            UniValue txinwitness(UniValue::VARR);
            txinwitness.reserve(tx.vin[i].scriptWitness.stack.size());
            for (const auto& item : tx.vin[i].scriptWitness.stack) {
                txinwitness.push_back(HexStr(item));
            }
            in.pushKV("txinwitness", std::move(txinwitness));
        }
        if (have_undo) {
            const Coin& prev_coin = txundo->vprevout[i];
            const CTxOut& prev_txout = prev_coin.out;

            amt_total_in += prev_txout.nValue;

            if (verbosity == TxVerbosity::SHOW_DETAILS_AND_PREVOUT) {
                UniValue o_script_pub_key(UniValue::VOBJ);
                ScriptToUniv(prev_txout.scriptPubKey, /*out=*/o_script_pub_key, /*include_hex=*/true, /*include_address=*/true);

                UniValue p(UniValue::VOBJ);
                p.pushKV("generated", bool(prev_coin.fCoinBase));
                p.pushKV("height", uint64_t(prev_coin.nHeight));
                p.pushKV("value", ValueFromAmount(prev_txout.nValue));
                p.pushKV("scriptPubKey", std::move(o_script_pub_key));
                in.pushKV("prevout", std::move(p));
            }
        }
        in.pushKV("sequence", (int64_t)txin.nSequence);
        vin.push_back(std::move(in));
    }
    entry.pushKV("vin", std::move(vin));

    UniValue vout(UniValue::VARR);
    vout.reserve(tx.vout.size());
    for (unsigned int i = 0; i < tx.vout.size(); i++) {
        const CTxOut& txout = tx.vout[i];

        UniValue out(UniValue::VOBJ);

        out.pushKV("value", ValueFromAmount(txout.nValue));
        out.pushKV("n", (int64_t)i);

        UniValue o(UniValue::VOBJ);
        ScriptToUniv(txout.scriptPubKey, /*out=*/o, /*include_hex=*/true, /*include_address=*/true);
        out.pushKV("scriptPubKey", std::move(o));
        vout.push_back(std::move(out));

        if (have_undo) {
            amt_total_out += txout.nValue;
        }
    }
    entry.pushKV("vout", std::move(vout));

    if (have_undo) {
        const CAmount fee = amt_total_in - amt_total_out;
        CHECK_NONFATAL(MoneyRange(fee));
        entry.pushKV("fee", ValueFromAmount(fee));
    }

    if (!block_hash.IsNull()) {
        entry.pushKV("blockhash", block_hash.GetHex());
    }

    if (include_hex) {
        entry.pushKV("hex", EncodeHexTx(tx)); // The hex-encoded transaction. Used the name "hex" to be consistent with the verbose output of "getrawtransaction".
    }
}
