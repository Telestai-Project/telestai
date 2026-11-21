// Copyright (c) 2026-present The Telestai Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/license/mit/.

#include <addresstype.h>
#include <assets/assets.h>
#include <assets/assettypes.h>
#include <chainparams.h>
#include <coins.h>
#include <consensus/amount.h>
#include <consensus/tx_verify.h>
#include <consensus/validation.h>
#include <key_io.h>
#include <primitives/transaction.h>
#include <script/script.h>
#include <test/util/setup_common.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <boost/test/unit_test.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

// The context-free asset rules of Telestai 2.1.x (CheckTransaction). 3.0.0 - 3.0.2 did not
// enforce any of them, so each case here is a transaction 2.1.x rejects and 3.0.2 accepted.

namespace {
const int HEIGHT{1};

PKHash Addr(unsigned char fill)
{
    return PKHash{uint160{std::vector<unsigned char>(20, fill)}};
}

CTxOut Pay(const PKHash& to, CAmount value = COIN)
{
    return CTxOut{value, GetScriptForDestination(to)};
}

CTxOut Transfer(const std::string& name, CAmount amount, const PKHash& to = Addr(0x11), CAmount value = 0)
{
    CScript script{GetScriptForDestination(to)};
    CAssetTransfer{name, amount}.ConstructTransaction(script);
    return CTxOut{value, script};
}

/** Qualifier tag (#NAME) or address restriction ($NAME) applied to an address. */
CTxOut AddressData(const std::string& name, int8_t flag, const PKHash& address = Addr(0x22))
{
    CScript script{CScript{} << OP_TLS_ASSET << ToByteVector(address)};
    CNullAssetTxData{name, flag}.ConstructTransaction(script);
    return CTxOut{0, script};
}

CTxOut GlobalRestriction(const std::string& name, int8_t flag)
{
    CScript script;
    CNullAssetTxData{name, flag}.ConstructGlobalRestrictionTransaction(script);
    return CTxOut{0, script};
}

CTxOut Verifier(const std::string& verifier)
{
    CScript script;
    CNullAssetTxVerifierString{verifier}.ConstructTransaction(script);
    return CTxOut{0, script};
}

CTxOut TagFee(int count, int height)
{
    const CTxDestination dest{DecodeDestination(GetBurnAddress(AssetType::NULL_ADD_QUALIFIER, height))};
    return CTxOut{GetBurnAmount(AssetType::NULL_ADD_QUALIFIER) * count, GetScriptForDestination(dest)};
}

CTxOut IssueAsset(const std::string& name, const PKHash& to, CAmount value = 0)
{
    CScript script{GetScriptForDestination(to)};
    CNewAsset{name, 21000 * COIN, /*units=*/0, /*nReissuable=*/1, /*nHasIPFS=*/0, /*strIPFSHash=*/""}.ConstructTransaction(script);
    return CTxOut{value, script};
}

CTxOut IssueOwner(const std::string& name, const PKHash& to)
{
    CScript script{GetScriptForDestination(to)};
    CNewAsset{name, 21000 * COIN}.ConstructOwnerTransaction(script);
    return CTxOut{0, script};
}

CMutableTransaction Tx(std::vector<CTxOut> outputs, bool coinbase = false)
{
    CMutableTransaction mtx;
    mtx.vin.resize(1);
    if (coinbase) {
        mtx.vin[0].prevout.SetNull();
        mtx.vin[0].scriptSig = CScript{} << 1 << OP_0;
    } else {
        mtx.vin[0].prevout = COutPoint{Txid::FromUint256(uint256::ONE), 0};
    }
    mtx.vout = std::move(outputs);
    return mtx;
}

/** Empty when the transaction passes, otherwise the reject reason. */
std::string Check(const CMutableTransaction& mtx, int height = HEIGHT)
{
    TxValidationState state;
    if (Consensus::CheckTxAssetStructure(CTransaction{mtx}, state, height)) return "";
    BOOST_CHECK(state.GetResult() == TxValidationResult::TX_CONSENSUS);
    return state.GetRejectReason();
}
} // namespace

BOOST_FIXTURE_TEST_SUITE(asset_tx_check_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(transfers)
{
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01))})), "");
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), Transfer("TLSARMY", 5 * COIN)})), "");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("TLSARMY!", 1 * COIN)})), "");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("TLSARMY#CARD", 1 * COIN)})), "");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 10 * COIN)})), "");

    // Asset outputs never carry TLS
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("TLSARMY", 5 * COIN, Addr(0x11), /*value=*/1)})),
                      "bad-txns-asset-transfer-amount-isn't-zero");

    BOOST_CHECK_EQUAL(Check(Tx({Transfer("TLSARMY!", 2 * COIN)})), "bad-txns-transfer-owner-amount-was-not-1");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("TLSARMY#CARD", 2 * COIN)})), "bad-txns-transfer-unique-amount-was-not-1");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 11 * COIN)})), "bad-txns-transfer-qualifier-amount-must be between 1 - 100");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 0)})), "bad-txns-transfer-qualifier-amount-must be between 1 - 100");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("not a name", 1 * COIN)})), "bad-txns-transfer-asset-name-invalid");
}

BOOST_AUTO_TEST_CASE(coinbase_carries_no_assets)
{
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01))}, /*coinbase=*/true)), "");

    // A coinbase has no inputs, so an asset output in it would create assets from nothing.
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), Transfer("TLSARMY", 5 * COIN)}, /*coinbase=*/true)),
                      "bad-txns-coinbase-contains-asset-txes");
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), Transfer("TLSARMY!", 1 * COIN)}, /*coinbase=*/true)),
                      "bad-txns-coinbase-contains-asset-txes");
    // The same output is fine in an ordinary transaction (balance is checked against its inputs).
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), Transfer("TLSARMY", 5 * COIN)})), "");
}

BOOST_AUTO_TEST_CASE(issuance)
{
    const PKHash issuer{Addr(0x31)};
    const PKHash other{Addr(0x32)};

    // fee, owner token, asset: the layout IsNewAsset() recognises
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), IssueOwner("TLSARMY", issuer), IssueAsset("TLSARMY", issuer)})), "");

    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), IssueOwner("TLSARMY", other), IssueAsset("TLSARMY", issuer)})),
                      "bad-txns-owner-address-mismatch");
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), IssueOwner("OTHERNAME", issuer), IssueAsset("TLSARMY", issuer)})),
                      "bad-txns-owner-name-mismatch");
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), IssueOwner("TLSARMY", issuer), IssueAsset("TLSARMY", issuer, /*value=*/1)})),
                      "bad-txns-asset-issued-amount-isn't-zero");

    // A new-asset output outside an issuance layout
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), IssueAsset("TLSARMY", issuer)})), "bad-txns-bad-asset-transaction");

    // OP_TLS_ASSET outside an asset or null asset data script
    BOOST_CHECK_EQUAL(Check(Tx({CTxOut{0, CScript{} << OP_1 << OP_TLS_ASSET}})),
                      "bad-txns-op-tls-asset-not-in-right-script-location");
}

BOOST_AUTO_TEST_CASE(qualifier_tags)
{
    // Adding a tag: transfer the qualifier, name the address, pay the fee
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 1 * COIN), AddressData("#KYC", 1), TagFee(1, HEIGHT)})), "");
    // Removing a tag is free
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 1 * COIN), AddressData("#KYC", 0)})), "");
    // Two addresses, two fees
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 1 * COIN), AddressData("#KYC", 1, Addr(0x22)), AddressData("#KYC", 1, Addr(0x23)), TagFee(2, HEIGHT)})), "");

    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 1 * COIN), AddressData("#KYC", 1)})),
                      "bad-txns-tx-doesn't-contain-required-burn-fee-for-adding-tags");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 1 * COIN), AddressData("#KYC", 1, Addr(0x22)), AddressData("#KYC", 1, Addr(0x23)), TagFee(1, HEIGHT)})),
                      "bad-txns-tx-doesn't-contain-required-burn-fee-for-adding-tags");
    BOOST_CHECK_EQUAL(Check(Tx({AddressData("#KYC", 1), TagFee(1, HEIGHT)})),
                      "bad-txns-tx-contains-qualifier-asset-null-tx-without-asset-transfer");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 1 * COIN), AddressData("#KYC", 1), AddressData("#KYC", 0), TagFee(1, HEIGHT)})),
                      "bad-txns-null-data-only-one-change-per-asset-address");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 1 * COIN), AddressData("#KYC", 2)})),
                      "bad-txns-null-data-flag-must-be-0-or-1");

    // The fee address is height dependent: from the 3.0.0 fork height the 2.1.x address no longer counts.
    const int fork_height{Params().GetConsensus().nCore300ActivationHeight};
    BOOST_CHECK(GetBurnAddress(AssetType::NULL_ADD_QUALIFIER, HEIGHT) != GetBurnAddress(AssetType::NULL_ADD_QUALIFIER, fork_height));
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 1 * COIN), AddressData("#KYC", 1), TagFee(1, HEIGHT)}), fork_height),
                      "bad-txns-tx-doesn't-contain-required-burn-fee-for-adding-tags");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("#KYC", 1 * COIN), AddressData("#KYC", 1), TagFee(1, fork_height)}), fork_height), "");
}

BOOST_AUTO_TEST_CASE(restrictions_and_verifiers)
{
    // Freezing an address or the whole asset needs the owner token (TOKEN!) in the same transaction
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("TOKEN!", 1 * COIN), AddressData("$TOKEN", 1)})), "");
    BOOST_CHECK_EQUAL(Check(Tx({AddressData("$TOKEN", 1)})),
                      "bad-txns-tx-contains-restricted-asset-null-tx-without-asset-transfer");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("$TOKEN", 1 * COIN), AddressData("$TOKEN", 1)})),
                      "bad-txns-tx-contains-restricted-asset-null-tx-without-asset-transfer");

    BOOST_CHECK_EQUAL(Check(Tx({Transfer("TOKEN!", 1 * COIN), GlobalRestriction("$TOKEN", 1)})), "");
    BOOST_CHECK_EQUAL(Check(Tx({GlobalRestriction("$TOKEN", 1)})),
                      "bad-txns-tx-contains-global-asset-null-tx-without-asset-transfer");
    BOOST_CHECK_EQUAL(Check(Tx({Transfer("TOKEN!", 1 * COIN), GlobalRestriction("$TOKEN", 1), GlobalRestriction("$TOKEN", 0)})),
                      "bad-txns-null-data-only-one-global-change-per-asset-name");

    // A verifier string belongs to a restricted asset issuance or reissuance, one per transaction
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), Verifier("KYC")})),
                      "bad-txns-tx-cointains-verifier-string-without-restricted-asset-issuance-or-reissuance");
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), Verifier("KYC"), Verifier("KYC")})),
                      "bad-txns-null-data-only-one-verifier-per-tx");
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), Verifier("KYC & AML")})),
                      "bad-txns-null-verifier-data-contained-whitespaces");
    BOOST_CHECK_EQUAL(Check(Tx({Pay(Addr(0x01)), Verifier("KYC")}, /*coinbase=*/true)),
                      "bad-txns-coinbase-contains-asset-txes");
}

namespace {
/** A coin database as 3.0.0 - 3.0.2 wrote it: it can hold outputs that are not coins. */
class StoredCoins : public CCoinsView
{
public:
    std::map<COutPoint, Coin> coins;
    std::optional<Coin> GetCoin(const COutPoint& outpoint) const override
    {
        const auto it = coins.find(outpoint);
        if (it == coins.end()) return std::nullopt;
        return it->second;
    }
    bool HaveCoin(const COutPoint& outpoint) const override { return coins.contains(outpoint); }
};
} // namespace

// 3.0.0 - 3.0.2 stored qualifier tags, verifier strings and zero-amount asset outputs as coins.
// A node upgraded in place still has them on disk; spending one must fail exactly as it does on
// a node that never stored it.
BOOST_AUTO_TEST_CASE(unspendable_coin_is_not_an_input)
{
    const Txid txid{Txid::FromUint256(uint256::ONE)};
    const COutPoint coin_out{txid, 0}, tag_out{txid, 1}, verifier_out{txid, 2}, zero_transfer_out{txid, 3}, transfer_out{txid, 4};

    StoredCoins disk;
    const auto store = [&](const COutPoint& out, const CTxOut& txout) { disk.coins.emplace(out, Coin{txout, /*nHeightIn=*/1, /*fCoinBaseIn=*/false}); };
    store(coin_out, Pay(Addr(0x01), 10 * COIN));
    store(tag_out, AddressData("#KYC", 1));
    store(verifier_out, Verifier("KYC"));
    store(zero_transfer_out, Transfer("TLSARMY", 0));
    store(transfer_out, Transfer("TLSARMY", 5 * COIN));
    CCoinsViewCache view{&disk};

    const auto check = [&](std::vector<COutPoint> inputs) -> std::string {
        CMutableTransaction mtx;
        for (const COutPoint& in : inputs) mtx.vin.emplace_back(in);
        mtx.vout = {Pay(Addr(0x02), 1 * COIN)};
        const CTransaction tx{mtx};
        BOOST_CHECK(view.HaveInputs(tx)); // the stored entry is found...
        TxValidationState state;
        CAmount fee{0};
        if (Consensus::CheckTxInputs(tx, state, view, /*nSpendHeight=*/200, fee)) return "";
        BOOST_CHECK(state.GetResult() == TxValidationResult::TX_CONSENSUS);
        return state.GetRejectReason();
    };

    BOOST_CHECK_EQUAL(check({coin_out}), "");
    BOOST_CHECK_EQUAL(check({coin_out, transfer_out}), "");
    // ...but it is not a coin
    BOOST_CHECK_EQUAL(check({coin_out, tag_out}), "bad-txns-inputs-missingorspent");
    BOOST_CHECK_EQUAL(check({coin_out, verifier_out}), "bad-txns-inputs-missingorspent");
    BOOST_CHECK_EQUAL(check({coin_out, zero_transfer_out}), "bad-txns-inputs-missingorspent");
    BOOST_CHECK_EQUAL(check({tag_out}), "bad-txns-inputs-missingorspent");
}

BOOST_AUTO_TEST_SUITE_END()
