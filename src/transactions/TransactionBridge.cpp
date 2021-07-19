// Copyright 2020 Stellar Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "transactions/TransactionBridge.h"
#include "transactions/TransactionFrame.h"

namespace stellar
{
namespace txbridge
{

TransactionEnvelope
convertForV13(TransactionEnvelope const& input)
{
    if (input.type() != ENVELOPE_TYPE_TX_V0)
    {
        return input;
    }

    auto const& envV0 = input.v0();
    auto const& txV0 = envV0.tx;

    TransactionEnvelope res(ENVELOPE_TYPE_TX);
    auto& envV1 = res.v1();
    auto& txV1 = envV1.tx;

    envV1.signatures = envV0.signatures;
    txV1.sourceAccount.ed25519() = txV0.sourceAccountEd25519;
    txV1.fee = txV0.fee;
    if (txV0.timeBounds)
    {
        txV1.cond.type(PRECOND_TIME);
        txV1.cond.timeBounds() = *txV0.timeBounds.get();
    }
    txV1.seqNum = txV0.seqNum;
    txV1.memo = txV0.memo;
    txV1.operations = txV0.operations;

    return res;
}

xdr::xvector<DecoratedSignature, 20>&
getSignatures(TransactionEnvelope& env)
{
    switch (env.type())
    {
    case ENVELOPE_TYPE_TX_V0:
        return env.v0().signatures;
    case ENVELOPE_TYPE_TX:
        return env.v1().signatures;
    case ENVELOPE_TYPE_TX_FEE_BUMP:
        return env.feeBump().signatures;
    default:
        abort();
    }
}

xdr::xvector<DecoratedSignature, 20>&
getSignaturesInner(TransactionEnvelope& env)
{
    switch (env.type())
    {
    case ENVELOPE_TYPE_TX_V0:
        return env.v0().signatures;
    case ENVELOPE_TYPE_TX:
        return env.v1().signatures;
    case ENVELOPE_TYPE_TX_FEE_BUMP:
        return env.feeBump().tx.innerTx.v1().signatures;
    default:
        abort();
    }
}

xdr::xvector<Operation, MAX_OPS_PER_TX>&
getOperations(TransactionEnvelope& env)
{
    switch (env.type())
    {
    case ENVELOPE_TYPE_TX_V0:
        return env.v0().tx.operations;
    case ENVELOPE_TYPE_TX:
        return env.v1().tx.operations;
    case ENVELOPE_TYPE_TX_FEE_BUMP:
        assert(env.feeBump().tx.innerTx.type() == ENVELOPE_TYPE_TX);
        return env.feeBump().tx.innerTx.v1().tx.operations;
    default:
        abort();
    }
}

#ifdef BUILD_TESTS
xdr::xvector<DecoratedSignature, 20>&
getSignatures(TransactionFramePtr tx)
{
    return getSignatures(tx->getEnvelope());
}

void
setSeqNum(TransactionFramePtr tx, int64_t seq)
{
    auto& env = tx->getEnvelope();
    int64_t& s = env.type() == ENVELOPE_TYPE_TX_V0 ? env.v0().tx.seqNum
                                                   : env.v1().tx.seqNum;
    s = seq;
}

void
setFee(TransactionFramePtr tx, uint32_t fee)
{
    auto& env = tx->getEnvelope();
    uint32_t& f =
        env.type() == ENVELOPE_TYPE_TX_V0 ? env.v0().tx.fee : env.v1().tx.fee;
    f = fee;
}

void
setMinTime(TransactionFramePtr tx, TimePoint minTime)
{
    auto& env = tx->getEnvelope();
    if (env.type() == ENVELOPE_TYPE_TX_V0)
    {
        env.v0().tx.timeBounds.activate().minTime = minTime;
    }
    else if (env.type() == ENVELOPE_TYPE_TX)
    {
        auto& cond = env.v1().tx.cond;
        switch (cond.type())
        {
        case PRECOND_NONE:
            cond.type(PRECOND_TIME);
        case PRECOND_TIME:
            cond.timeBounds().minTime = minTime;
            break;
        case PRECOND_GENERAL:
            cond.general().timeBounds.activate().minTime = minTime;
            break;
        }
    }
}

void
setMaxTime(TransactionFramePtr tx, TimePoint maxTime)
{
    auto& env = tx->getEnvelope();
    if (env.type() == ENVELOPE_TYPE_TX_V0)
    {
        env.v0().tx.timeBounds.activate().maxTime = maxTime;
    }
    else if (env.type() == ENVELOPE_TYPE_TX)
    {
        auto& cond = env.v1().tx.cond;
        switch (cond.type())
        {
        case PRECOND_NONE:
            cond.type(PRECOND_TIME);
        case PRECOND_TIME:
            cond.timeBounds().maxTime = maxTime;
            break;
        case PRECOND_GENERAL:
            cond.general().timeBounds.activate().maxTime = maxTime;
            break;
        }
    }
}

void
setGeneralPrecond(TransactionFramePtr tx, GeneralPreconditions general)
{
    auto& env = tx->getEnvelope();
    auto& cond = env.v1().tx.cond;
    cond.type(PRECOND_GENERAL);
    cond.general() = general;
}
#endif
}
}
