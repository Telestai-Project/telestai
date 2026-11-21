// Copyright (c) 2026-present The Telestai Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/license/mit/.

#include <addresstype.h>
#include <assets/assets.h>
#include <assets/assettypes.h>
#include <base58.h>
#include <coins.h>
#include <consensus/amount.h>
#include <core_io.h>
#include <key_io.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <script/script_error.h>
#include <script/solver.h>
#include <span.h>
#include <test/util/setup_common.h>
#include <uint256.h>
#include <univalue.h>
#include <util/strencodings.h>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace {
PKHash Owner()
{
    return PKHash{uint160{ParseHex("00112233445566778899aabbccddeeff00112233")}};
}

/** 0x12 0x20 followed by a 32-byte digest: the raw form of a "Qm..." IPFS hash. */
std::string RawIpfsHash()
{
    std::string raw{"\x12\x20"};
    raw.append(32, '\x5a');
    return raw;
}

UniValue Describe(const CScript& script)
{
    UniValue out{UniValue::VOBJ};
    ScriptToUniv(script, out, /*include_hex=*/true, /*include_address=*/true);
    return out;
}
} // namespace

BOOST_FIXTURE_TEST_SUITE(asset_script_tests, BasicTestingSetup)

// Regression test: 3.0.0 - 3.0.2 resolved every asset output to "no destination", so the
// asset database, the address index and the transaction RPCs all saw an empty address.
BOOST_AUTO_TEST_CASE(asset_script_destination)
{
    const PKHash owner{Owner()};
    const std::string owner_address{EncodeDestination(owner)};

    CScript script{GetScriptForDestination(owner)};
    CAssetTransfer{"TLSARMY", 5 * COIN}.ConstructTransaction(script);
    BOOST_CHECK(script.IsAssetScript());

    CTxDestination dest;
    BOOST_CHECK(ExtractDestination(script, dest));
    BOOST_CHECK(dest == CTxDestination{owner});
    // The address only identifies the owner; it does not reproduce the asset script.
    // Code that rebuilds outputs (fundrawtransaction, bumpfee) has to keep the script.
    BOOST_CHECK(GetScriptForDestination(dest) != script);

    CAssetTransfer transfer;
    std::string address;
    BOOST_CHECK(TransferAssetFromScript(script, transfer, address));
    BOOST_CHECK_EQUAL(address, owner_address);
    BOOST_CHECK_EQUAL(transfer.strName, "TLSARMY");

    CAssetOutputEntry data;
    BOOST_CHECK(GetAssetData(script, data));
    BOOST_CHECK(data.destination == CTxDestination{owner});

    // Every kind of asset script resolves to its owner.
    for (const bool owner_token : {false, true}) {
        CScript issue{GetScriptForDestination(owner)};
        const CNewAsset asset{"TLSARMY", 21000 * COIN};
        owner_token ? asset.ConstructOwnerTransaction(issue) : asset.ConstructTransaction(issue);
        BOOST_CHECK(ExtractDestination(issue, dest));
        BOOST_CHECK(dest == CTxDestination{owner});
    }
    CScript reissue{GetScriptForDestination(owner)};
    CReissueAsset{"TLSARMY", 100 * COIN, /*nUnits=*/-1, /*nReissuable=*/1, /*strIPFSHash=*/"", /*strANSID=*/""}.ConstructTransaction(reissue);
    BOOST_CHECK(ExtractDestination(reissue, dest));
    BOOST_CHECK(dest == CTxDestination{owner});

    // Scripts without an asset payload are unaffected.
    const CScript null_data{CScript{} << OP_RETURN << std::vector<unsigned char>{1, 2, 3}};
    BOOST_CHECK(!ExtractDestination(null_data, dest));
    BOOST_CHECK(GetScriptForDestination(dest) == null_data);
}

BOOST_AUTO_TEST_CASE(asset_script_to_univ)
{
    const PKHash owner{Owner()};
    const std::string owner_address{EncodeDestination(owner)};
    const std::string ipfs{RawIpfsHash()};
    const std::string ipfs_encoded{EncodeBase58(MakeUCharSpan(ipfs))};
    BOOST_CHECK_EQUAL(ipfs_encoded.size(), 46U);
    BOOST_CHECK_EQUAL(ipfs_encoded.substr(0, 2), "Qm");

    // Transfer with a message and expiry
    {
        CScript script{GetScriptForDestination(owner)};
        CAssetTransfer{"TLSARMY", 5 * COIN, ipfs, 1800000000}.ConstructTransaction(script);
        const UniValue out{Describe(script)};
        BOOST_CHECK_EQUAL(out["type"].get_str(), "transfer_asset");
        BOOST_CHECK_EQUAL(out["address"].get_str(), owner_address);
        const UniValue& asset{out["asset"]};
        BOOST_CHECK_EQUAL(asset["name"].get_str(), "TLSARMY");
        BOOST_CHECK_EQUAL(asset["amount"].getValStr(), "5.00000000");
        BOOST_CHECK_EQUAL(asset["message"].get_str(), ipfs_encoded);
        BOOST_CHECK_EQUAL(asset["expire_time"].getInt<int64_t>(), 1800000000);
        BOOST_CHECK(!asset.exists("units"));
    }

    // Plain transfer
    {
        CScript script{GetScriptForDestination(owner)};
        CAssetTransfer{"TLSARMY", COIN / 2}.ConstructTransaction(script);
        const UniValue asset = Describe(script)["asset"];
        BOOST_CHECK_EQUAL(asset["amount"].getValStr(), "0.50000000");
        BOOST_CHECK(!asset.exists("message"));
        BOOST_CHECK(!asset.exists("expire_time"));
    }

    // Issuance
    {
        CScript script{GetScriptForDestination(owner)};
        CNewAsset{"TLSARMY", 21000 * COIN, /*units=*/2, /*nReissuable=*/1, /*nHasIPFS=*/1, ipfs}.ConstructTransaction(script);
        const UniValue out{Describe(script)};
        BOOST_CHECK_EQUAL(out["type"].get_str(), "new_asset");
        BOOST_CHECK_EQUAL(out["address"].get_str(), owner_address);
        const UniValue& asset{out["asset"]};
        BOOST_CHECK_EQUAL(asset["name"].get_str(), "TLSARMY");
        BOOST_CHECK_EQUAL(asset["amount"].getValStr(), "21000.00000000");
        BOOST_CHECK_EQUAL(asset["units"].getInt<int>(), 2);
        BOOST_CHECK(asset["reissuable"].get_bool());
        BOOST_CHECK_EQUAL(asset["ipfs_hash"].get_str(), ipfs_encoded);
    }

    // Owner token
    {
        CScript script{GetScriptForDestination(owner)};
        CNewAsset{"TLSARMY", 21000 * COIN}.ConstructOwnerTransaction(script);
        const UniValue out{Describe(script)};
        BOOST_CHECK_EQUAL(out["type"].get_str(), "new_asset");
        BOOST_CHECK_EQUAL(out["address"].get_str(), owner_address);
        const UniValue& asset{out["asset"]};
        BOOST_CHECK_EQUAL(asset["name"].get_str(), "TLSARMY!");
        BOOST_CHECK_EQUAL(asset["amount"].getValStr(), "1.00000000");
        BOOST_CHECK(!asset.exists("units"));
        BOOST_CHECK(!asset.exists("reissuable"));
    }

    // Reissue that leaves the units unchanged
    {
        CScript script{GetScriptForDestination(owner)};
        CReissueAsset{"TLSARMY", 100 * COIN, /*nUnits=*/-1, /*nReissuable=*/0, /*strIPFSHash=*/"", /*strANSID=*/""}.ConstructTransaction(script);
        const UniValue out{Describe(script)};
        BOOST_CHECK_EQUAL(out["type"].get_str(), "reissue_asset");
        BOOST_CHECK_EQUAL(out["address"].get_str(), owner_address);
        const UniValue& asset{out["asset"]};
        BOOST_CHECK_EQUAL(asset["name"].get_str(), "TLSARMY");
        BOOST_CHECK_EQUAL(asset["amount"].getValStr(), "100.00000000");
        BOOST_CHECK(!asset.exists("units"));
        BOOST_CHECK(!asset["reissuable"].get_bool());
        BOOST_CHECK(!asset.exists("ipfs_hash"));
    }

    // Ordinary outputs get no asset fields.
    {
        const UniValue out{Describe(GetScriptForDestination(owner))};
        BOOST_CHECK_EQUAL(out["type"].get_str(), "pubkeyhash");
        BOOST_CHECK(!out.exists("asset"));
        BOOST_CHECK(!out.exists("asset_data"));
    }
}

BOOST_AUTO_TEST_CASE(asset_null_data_to_univ)
{
    const PKHash owner{Owner()};
    const std::string owner_address{EncodeDestination(owner)};

    // Qualifier tag on an address
    {
        CScript script{CScript{} << OP_TLS_ASSET << ToByteVector(owner)};
        CNullAssetTxData{"#KYC", 1}.ConstructTransaction(script);
        BOOST_CHECK(script.IsNullAssetTxDataScript());

        CTxDestination dest;
        BOOST_CHECK(ExtractDestination(script, dest));
        BOOST_CHECK(dest == CTxDestination{owner});

        CNullAssetTxData data;
        std::string address;
        BOOST_CHECK(AssetNullDataFromScript(script, data, address));
        BOOST_CHECK_EQUAL(address, owner_address);

        const UniValue out{Describe(script)};
        BOOST_CHECK_EQUAL(out["type"].get_str(), "restricted_asset_data");
        BOOST_CHECK_EQUAL(out["address"].get_str(), owner_address);
        const UniValue& asset_data{out["asset_data"]};
        BOOST_CHECK_EQUAL(asset_data["asset_name"].get_str(), "#KYC");
        BOOST_CHECK_EQUAL(asset_data["qualifier_type"].get_str(), "adding qualifier");
        BOOST_CHECK_EQUAL(asset_data["address"].get_str(), owner_address);
    }

    // Address restriction
    {
        CScript script{CScript{} << OP_TLS_ASSET << ToByteVector(owner)};
        CNullAssetTxData{"$TOKEN", 0}.ConstructTransaction(script);
        const UniValue asset_data = Describe(script)["asset_data"];
        BOOST_CHECK_EQUAL(asset_data["asset_name"].get_str(), "$TOKEN");
        BOOST_CHECK_EQUAL(asset_data["restricted_type"].get_str(), "unfreezing address");
        BOOST_CHECK_EQUAL(asset_data["address"].get_str(), owner_address);
    }

    // Global restriction: no address involved
    {
        CScript script;
        CNullAssetTxData{"$TOKEN", 1}.ConstructGlobalRestrictionTransaction(script);
        CTxDestination dest;
        BOOST_CHECK(!ExtractDestination(script, dest));
        BOOST_CHECK(GetScriptForDestination(dest) == script);

        const UniValue out{Describe(script)};
        BOOST_CHECK_EQUAL(out["type"].get_str(), "restricted_asset_data");
        BOOST_CHECK(!out.exists("address"));
        const UniValue& asset_data{out["asset_data"]};
        BOOST_CHECK_EQUAL(asset_data["restricted_name"].get_str(), "$TOKEN");
        BOOST_CHECK_EQUAL(asset_data["restricted_type"].get_str(), "freezing");
        BOOST_CHECK_EQUAL(asset_data["address"].get_str(), "all addresses");
    }

    // Verifier string
    {
        CScript script;
        CNullAssetTxVerifierString{"#KYC"}.ConstructTransaction(script);
        const UniValue out{Describe(script)};
        BOOST_CHECK(!out.exists("address"));
        BOOST_CHECK_EQUAL(out["asset_data"]["verifier_string"].get_str(), "#KYC");
    }
}

// Regression test: 3.0.0 - 3.0.2 kept these outputs in the UTXO set, where a qualifier tag
// could be spent by anyone. 2.1.x never creates a coin for them.
BOOST_AUTO_TEST_CASE(asset_scripts_unspendable)
{
    const PKHash owner{Owner()};
    const auto with_payload = [&](auto&& construct) {
        CScript script{GetScriptForDestination(owner)};
        construct(script);
        return script;
    };

    // Null asset data: everything that starts with OP_TLS_ASSET
    CScript tag{CScript{} << OP_TLS_ASSET << ToByteVector(owner)};
    CNullAssetTxData{"#KYC", 1}.ConstructTransaction(tag);
    CScript global;
    CNullAssetTxData{"$TOKEN", 1}.ConstructGlobalRestrictionTransaction(global);
    CScript verifier;
    CNullAssetTxVerifierString{"KYC"}.ConstructTransaction(verifier);
    BOOST_CHECK(tag.IsUnspendable());
    BOOST_CHECK(global.IsUnspendable());
    BOOST_CHECK(verifier.IsUnspendable());
    BOOST_CHECK((CScript{} << OP_TLS_ASSET).IsUnspendable());

    // Asset outputs are coins unless they carry nothing
    const CScript transfer{with_payload([](CScript& s) { CAssetTransfer{"TLSARMY", 5 * COIN}.ConstructTransaction(s); })};
    const CScript transfer_zero{with_payload([](CScript& s) { CAssetTransfer{"TLSARMY", 0}.ConstructTransaction(s); })};
    const CScript issue{with_payload([](CScript& s) { CNewAsset{"TLSARMY", 21000 * COIN}.ConstructTransaction(s); })};
    const CScript issue_zero{with_payload([](CScript& s) { CNewAsset{"TLSARMY", 0}.ConstructTransaction(s); })};
    const CScript owner_token{with_payload([](CScript& s) { CNewAsset{"TLSARMY", 0}.ConstructOwnerTransaction(s); })};
    const CScript reissue{with_payload([](CScript& s) { CReissueAsset{"TLSARMY", 100 * COIN, -1, 1, "", ""}.ConstructTransaction(s); })};
    const CScript reissue_zero{with_payload([](CScript& s) { CReissueAsset{"TLSARMY", 0, 4, 1, "", ""}.ConstructTransaction(s); })};
    BOOST_CHECK(!transfer.IsUnspendable());
    BOOST_CHECK(!issue.IsUnspendable());
    BOOST_CHECK(!owner_token.IsUnspendable()); // an owner token is always one unit
    BOOST_CHECK(!reissue.IsUnspendable());
    BOOST_CHECK(transfer_zero.IsUnspendable());
    BOOST_CHECK(issue_zero.IsUnspendable());
    BOOST_CHECK(reissue_zero.IsUnspendable()); // a reissue that only changes metadata

    // Unchanged for everything else
    BOOST_CHECK(!GetScriptForDestination(owner).IsUnspendable());
    BOOST_CHECK((CScript{} << OP_RETURN << std::vector<unsigned char>{1, 2, 3}).IsUnspendable());
    BOOST_CHECK(!CScript{}.IsUnspendable());

    // ...and the UTXO set follows the rule
    CCoinsView base;
    CCoinsViewCache view{&base};
    const Txid txid{Txid::FromUint256(uint256::ONE)};
    const std::vector<CScript> scripts{tag, verifier, transfer_zero, transfer, issue};
    for (size_t i = 0; i < scripts.size(); ++i) {
        view.AddCoin(COutPoint{txid, static_cast<uint32_t>(i)}, Coin{CTxOut{0, scripts[i]}, /*nHeightIn=*/1, /*fCoinBaseIn=*/false}, /*possible_overwrite=*/false);
    }
    BOOST_CHECK(!view.HaveCoin(COutPoint{txid, 0}));
    BOOST_CHECK(!view.HaveCoin(COutPoint{txid, 1}));
    BOOST_CHECK(!view.HaveCoin(COutPoint{txid, 2}));
    BOOST_CHECK(view.HaveCoin(COutPoint{txid, 3}));
    BOOST_CHECK(view.HaveCoin(COutPoint{txid, 4}));
}

// Regression test: 3.0.0 - 3.0.2 stopped evaluating a script at OP_TLS_ASSET. 2.1.x treats the
// opcode as a no-op and keeps going, so the two disagreed about scripts with anything other
// than the usual `<payload> OP_DROP` after it.
BOOST_AUTO_TEST_CASE(asset_opcode_is_noop)
{
    const auto eval = [](const CScript& script, ScriptError& err) {
        std::vector<std::vector<unsigned char>> stack;
        const bool ok{EvalScript(stack, script, SCRIPT_VERIFY_NONE, BaseSignatureChecker{}, SigVersion::BASE, &err)};
        // true when the script ran to completion and left a non-zero value on top of the stack
        return ok && !stack.empty() && std::ranges::any_of(stack.back(), [](unsigned char b) { return b != 0; });
    };
    const std::vector<unsigned char> payload{'r', 'v', 'n', 't', 1, 2, 3};
    ScriptError err;

    // The normal tail of an asset script leaves the stack as the P2PKH part left it.
    BOOST_CHECK(eval(CScript{} << OP_1 << OP_TLS_ASSET << payload << OP_DROP, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_OK);
    BOOST_CHECK(eval(CScript{} << OP_1 << OP_TLS_ASSET, err));

    // Whatever follows the opcode is executed, not skipped.
    BOOST_CHECK(!eval(CScript{} << OP_1 << OP_TLS_ASSET << payload << OP_DROP << OP_RETURN, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_OP_RETURN);
    BOOST_CHECK(!eval(CScript{} << OP_1 << OP_TLS_ASSET << payload << OP_DROP << OP_DROP, err));
    BOOST_CHECK(!eval(CScript{} << OP_1 << OP_TLS_ASSET << OP_0, err));
    BOOST_CHECK(!eval(CScript{} << OP_1 << OP_TLS_ASSET << OP_IF, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_UNBALANCED_CONDITIONAL);

    // In a branch that is not taken it does nothing, like any other opcode.
    BOOST_CHECK(eval(CScript{} << OP_1 << OP_0 << OP_IF << OP_TLS_ASSET << OP_RETURN << OP_ENDIF, err));
}

BOOST_AUTO_TEST_SUITE_END()
