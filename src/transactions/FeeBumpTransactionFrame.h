// Copyright 2020 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "transactions/TransactionFrame.h"

namespace stellar
{
class AbstractLedgerTxn;
class Application;
class SignatureChecker;

class FeeBumpTransactionFrame : public TransactionFrameBase
{
    TransactionEnvelope mEnvelope;
    TransactionResult mResult;

    TransactionFramePtr mInnerTx;

    mutable Hash mContentsHash;
    mutable Hash mFullHash;

    bool checkSignature(SignatureChecker& signatureChecker,
                        LedgerTxnEntry const& account, int32_t neededWeight);

    bool commonValidPreSourceAccountLoad(AbstractLedgerTxn& ltx);

    enum ValidationType
    {
        kInvalid,         // transaction is not valid at all
        kInvalidPostAuth, // transaction is invalid but one-time signers
                          // should be removed
        kFullyValid
    };

    // If "fullCheck" is false, then a "kInvalid" return definitively
    // establishes the invalidity of the transaction, but a "kFullyValid" return
    // does not definitively establish that a call with a true "fullCheck"
    // parameter would have returned "kFullyValid".  In particular, if
    // "fullCheck" is false, then commonValid() will not perform any checks that
    // could require checking signatures or accessing the database.
    ValidationType commonValid(SignatureChecker& signatureChecker,
                               AbstractLedgerTxn& ltxOuter, bool applying,
                               bool fullCheck);

    void removeOneTimeSignerKeyFromFeeSource(AbstractLedgerTxn& ltx) const;

  protected:
    void resetResults(LedgerHeader const& header, int64_t baseFee,
                      bool applying);

  public:
    FeeBumpTransactionFrame(Hash const& networkID,
                            TransactionEnvelope const& envelope);
#ifdef BUILD_TESTS
    FeeBumpTransactionFrame(TransactionEnvelope const& envelope,
                            TransactionFramePtr innerTx);
#endif

    virtual ~FeeBumpTransactionFrame(){};

    bool apply(Application& app, AbstractLedgerTxn& ltx,
               TransactionMeta& meta) override;

    bool checkValid(AbstractLedgerTxn& ltxOuter, SequenceNumber current,
                    uint64_t lowerBoundCloseTimeOffset,
                    uint64_t upperBoundCloseTimeOffset,
                    bool fullCheck) override;

    TransactionEnvelope const& getEnvelope() const override;

    int64_t getFeeBid() const override;
    int64_t getMinFee(LedgerHeader const& header) const override;
    int64_t getFee(LedgerHeader const& header, int64_t baseFee,
                   bool applying) const override;

    Hash const& getContentsHash() const override;
    Hash const& getFullHash() const override;
    Hash const& getInnerFullHash() const;

    Hash const& getNetworkID() const override;

    uint32_t getNumOperations() const override;

    TransactionResult& getResult() override;
    TransactionResultCode getResultCode() const override;

    SequenceNumber getSeqNum() const override;
    AccountID getFeeSourceID() const override;
    AccountID getSourceID() const override;

    void
    insertKeysForFeeProcessing(UnorderedSet<LedgerKey>& keys) const override;
    void insertKeysForTxApply(UnorderedSet<LedgerKey>& keys) const override;

    void processFeeSeqNum(AbstractLedgerTxn& ltx, int64_t baseFee) override;

    StellarMessage toStellarMessage() const override;

    static TransactionEnvelope
    convertInnerTxToV1(TransactionEnvelope const& envelope);
};
}
