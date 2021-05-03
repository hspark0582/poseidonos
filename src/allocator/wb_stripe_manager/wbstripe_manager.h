/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <string>
#include <vector>

#include "src/allocator/block_manager/block_manager.h"
#include "src/allocator/context_manager/context_manager.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator/i_wbstripe_internal.h"
#include "src/mapper/i_stripemap.h"

namespace pos
{
using StripeVec = std::vector<Stripe*>;

class WBStripeManager : public IWBStripeAllocator, public IWBStripeInternal
{
public:
    WBStripeManager(AllocatorAddressInfo* info, ContextManager* ctxMgr, BlockManager* blkMgr, std::string arrayName);
    virtual ~WBStripeManager(void);
    void Init(void);

    Stripe* GetStripe(StripeAddr& lsidEntry) override;
    StripeId AllocateUserDataStripeId(StripeId vsid) override;
    void FreeWBStripeId(StripeId lsid) override;

    void GetAllActiveStripes(uint32_t volumeId) override;
    bool WaitPendingWritesOnStripes(uint32_t volumeId) override;
    bool WaitStripesFlushCompletion(uint32_t volumeId) override;

    bool ReferLsidCnt(StripeAddr& lsa) override;
    void DereferLsidCnt(StripeAddr& lsa, uint32_t blockCount) override;
    void FlushAllActiveStripes(void) override;

    int ReconstructActiveStripe(uint32_t volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa, ASTailArrayIdx idx) override;
    Stripe* FinishReconstructedStripe(StripeId wbLsid, VirtualBlkAddr tail) override;
    int RestoreActiveStripeTail(int index, VirtualBlkAddr tail, StripeId wbLsid) override;
    int FlushPendingActiveStripes(void) override;

    int PrepareRebuild(void) override;
    int StopRebuilding(void) override;

    Stripe* GetStripe(StripeId wbLsid) override;

    void PickActiveStripe(uint32_t volumeId, std::vector<Stripe*>& stripesToFlush, std::vector<StripeId>& vsidToCheckFlushDone);
    void FinalizeWriteIO(std::vector<Stripe*>& stripesToFlush, std::vector<StripeId>& vsidToCheckFlushDone);
    int CheckAllActiveStripes(std::vector<Stripe*>& stripesToFlush, std::vector<StripeId>& vsidToCheckFlushDone);

private:
    int _MakeRebuildTarget(void);
    int _FlushOnlineStripes(std::vector<StripeId>& vsidToCheckFlushDone);
    Stripe* _FinishActiveStripe(ASTailArrayIdx index);
    VirtualBlks _AllocateRemainingBlocks(ASTailArrayIdx index);
    VirtualBlks _AllocateRemainingBlocks(VirtualBlkAddr tail);
    Stripe* _FinishRemainingBlocks(VirtualBlks remainingVsaRange);
    int _RequestStripeFlush(Stripe& stripe);
    int _ReconstructAS(StripeId vsid, StripeId wbLsid, uint64_t blockCount, ASTailArrayIdx idx, Stripe*& stripe);
    int _ReconstructReverseMap(uint32_t volumeId, Stripe* stripe, uint64_t blockCount);

    std::vector<Stripe*> wbStripeArray;
    FreeBufferPool* stripeBufferPool;

    std::vector<Stripe*> stripesToFlush4FlushCmd[MAX_VOLUME_COUNT];
    std::vector<StripeId> vsidToCheckFlushDone4FlushCmd[MAX_VOLUME_COUNT];

    StripeVec* pendingFullStripes;

    // DOCs
    IStripeMap* iStripeMap;
    AllocatorAddressInfo* addrInfo;
    ContextManager* contextManager;
    WbStripeCtx* wbStripeCtx;
    BlockManager* blockManager;
    std::string arrayName;
};

} // namespace pos
