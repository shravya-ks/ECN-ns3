/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Shravya Ks <shravya.ks0@gmail.com>
 *
 */

#include "ns3/test.h"
#include "ns3/red-queue-disc.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/ipv6-queue-disc-item.h"
#include "ns3/rng-seed-manager.h"

using namespace ns3;

class EcnIpv6RedQueueDiscTestItem : public Ipv6QueueDiscItem
{
public:
  EcnIpv6RedQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol, const Ipv6Header & header);
  virtual ~EcnIpv6RedQueueDiscTestItem ();

private:
  EcnIpv6RedQueueDiscTestItem ();
  EcnIpv6RedQueueDiscTestItem (const EcnIpv6RedQueueDiscTestItem &);
  EcnIpv6RedQueueDiscTestItem &operator = (const EcnIpv6RedQueueDiscTestItem &);
};

EcnIpv6RedQueueDiscTestItem::EcnIpv6RedQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol, const Ipv6Header & header)
  : Ipv6QueueDiscItem (p, addr, protocol, header)
{
}

EcnIpv6RedQueueDiscTestItem::~EcnIpv6RedQueueDiscTestItem ()
{
}

class EcnIpv6RedQueueDiscTestCase : public TestCase
{
public:
  EcnIpv6RedQueueDiscTestCase ();
  virtual void DoRun (void);
private:
  void Enqueue (Ptr<RedQueueDisc> queue, uint32_t size, uint32_t nPkt);
  void EnqueueNonEcnCapable (Ptr<RedQueueDisc> queue, uint32_t size, uint32_t nPkt);
  void RunRedTest (StringValue mode);
};

EcnIpv6RedQueueDiscTestCase::EcnIpv6RedQueueDiscTestCase ()
  : TestCase ("Sanity check on the Ecn implementation for Ipv6 header for red queue")
{
}

void
EcnIpv6RedQueueDiscTestCase::RunRedTest (StringValue mode)
{
  uint32_t pktSize = 0;
  // 1 for packets; pktSize for bytes
  uint32_t modeSize = 1;
  double minTh = 2;
  double maxTh = 5;
  uint32_t qSize = 8;
  Ptr<RedQueueDisc> queue = CreateObject<RedQueueDisc> ();
  queue->AssignStreams (1);

  // test 1: simple enqueue/dequeue with no drops or marks
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MinTh", DoubleValue (minTh)), true,
                         "Verify that we can actually set the attribute MinTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MaxTh", DoubleValue (maxTh)), true,
                         "Verify that we can actually set the attribute MaxTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QW", DoubleValue (0.002)), true,
                         "Verify that we can actually set the attribute QW");

  Address dest;

  if (queue->GetMode () == Queue::QUEUE_MODE_BYTES)
    {
      pktSize = 1000;
      modeSize = pktSize + 40;
      queue->SetTh (minTh * modeSize, maxTh * modeSize);
      queue->SetQueueLimit (qSize * modeSize);
    }

  Ipv6Header hdr, tmp_hdr;
  hdr.SetEcn (Ipv6Header::ECN_ECT0);

  Ptr<Packet> p1, p2, p3, p4, p5, p6, p7, p8;
  p1 = Create<Packet> (pktSize);

  p2 = Create<Packet> (pktSize);

  p3 = Create<Packet> (pktSize);

  p4 = Create<Packet> (pktSize);

  p5 = Create<Packet> (pktSize);

  queue->Initialize ();

  Ptr<QueueDiscItem> item;

  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 0 * modeSize, "There should be no packets in there");
  item = Create<EcnIpv6RedQueueDiscTestItem> (p1, dest, 0, hdr);
  queue->Enqueue (item);
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 1 * modeSize, "There should be one packet in there");
  item = Create<EcnIpv6RedQueueDiscTestItem> (p2, dest, 0, hdr);  
  queue->Enqueue (item);
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 2 * modeSize, "There should be two packets in there");
  item = Create<EcnIpv6RedQueueDiscTestItem> (p3, dest, 0, hdr);
  queue->Enqueue (item);
  item = Create<EcnIpv6RedQueueDiscTestItem> (p4, dest, 0, hdr);
  queue->Enqueue (item);
  item = Create<EcnIpv6RedQueueDiscTestItem> (p5, dest, 0, hdr);
  queue->Enqueue (item);
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 5 * modeSize, "There should be five packets in there");

  item = DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the first packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 4 * modeSize, "There should be four packets in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p1->GetUid (), "was this the first packet ?");

  NS_TEST_EXPECT_MSG_EQ ((DynamicCast<Ipv6QueueDiscItem>(item)->GetHeader()).GetEcn (), Ipv6Header::ECN_ECT0, "The packet should be marked");
   
  item = DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the second packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 3 * modeSize, "There should be three packet in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p2->GetUid (), "Was this the second packet ?");

  NS_TEST_EXPECT_MSG_EQ ((DynamicCast<Ipv6QueueDiscItem>(item)->GetHeader()).GetEcn (), Ipv6Header::ECN_ECT0, "The packet should be marked");
   
  item = DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the third packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 2 * modeSize, "There should be two packets in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p3->GetUid (), "Was this the third packet ?");

  DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());
  DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());

  item = DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item == 0), true, "There are really no packets in there");

  //test2 : Test with packets which are not ECN-Capable

  queue = CreateObject<RedQueueDisc> ();

  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MinTh", DoubleValue (minTh)), true,
                         "Verify that we can actually set the attribute MinTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MaxTh", DoubleValue (maxTh)), true,
                         "Verify that we can actually set the attribute MaxTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QW", DoubleValue (0.002)), true,
                         "Verify that we can actually set the attribute QW");

  if (queue->GetMode () == Queue::QUEUE_MODE_BYTES)
    {
      pktSize = 1000;
      modeSize = pktSize + 40;
      queue->SetTh (minTh * modeSize, maxTh * modeSize);
      queue->SetQueueLimit (qSize * modeSize);
    }

  p1 = Create<Packet> (pktSize);

  p2 = Create<Packet> (pktSize);

  p3 = Create<Packet> (pktSize);

  p4 = Create<Packet> (pktSize);

  p5 = Create<Packet> (pktSize);

  Ipv6Header hdr1;

  queue->Initialize ();
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 0 * modeSize, "There should be no packets in there");
  item = Create<EcnIpv6RedQueueDiscTestItem> (p1, dest, 0, hdr1);
  
  queue->Enqueue (item);
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 1 * modeSize, "There should be one packet in there");
  item = Create<EcnIpv6RedQueueDiscTestItem> (p2, dest, 0, hdr1);
  
  queue->Enqueue (item);
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 2 * modeSize, "There should be two packets in there");
  item = Create<EcnIpv6RedQueueDiscTestItem> (p3, dest, 0, hdr1);
  
  queue->Enqueue (item);
  item = Create<EcnIpv6RedQueueDiscTestItem> (p4, dest, 0, hdr1);
  
  queue->Enqueue (item);
  item = Create<EcnIpv6RedQueueDiscTestItem> (p5, dest, 0, hdr1);
  
  queue->Enqueue (item);
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 5 * modeSize, "There should be five packets in there");

  item = DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the first packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 4 * modeSize, "There should be four packets in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p1->GetUid (), "was this the first packet ?");

  if (item->GetPacket ()->GetSize ())
    {
      item->GetPacket ()->PeekHeader (tmp_hdr);
      NS_TEST_EXPECT_MSG_EQ (tmp_hdr.GetEcn (), Ipv6Header::ECN_NotECT, "The packet should not be marked");
    }

  item = DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the second packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 3 * modeSize, "There should be  three packets in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p2->GetUid (), "Was this the second packet ?");

  if (item->GetPacket ()->GetSize ())
    {
      item->GetPacket ()->PeekHeader (tmp_hdr);
      NS_TEST_EXPECT_MSG_EQ (tmp_hdr.GetEcn (), Ipv6Header::ECN_NotECT, "The packet should not be marked");
    }

  item = DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the third packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 2 * modeSize, "There should be two packets in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p3->GetUid (), "Was this the third packet ?");

  DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());
  DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());
  item = DynamicCast<Ipv6QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item == 0), true, "There are really no packets in there");


  // test 3: more data, but with no marks or drops
  queue = CreateObject<RedQueueDisc> ();
  minTh = 70 * modeSize;
  maxTh = 150 * modeSize;
  qSize = 300 * modeSize;
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MinTh", DoubleValue (minTh)), true,
                         "Verify that we can actually set the attribute MinTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MaxTh", DoubleValue (maxTh)), true,
                         "Verify that we can actually set the attribute MaxTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  queue->Initialize ();
  Enqueue (queue, pktSize, 300);
  RedQueueDisc::Stats st = queue->GetStats ();
  NS_TEST_EXPECT_MSG_EQ (st.unforcedMark, 0, "There should zero marked packets due probability mark");
  NS_TEST_EXPECT_MSG_EQ (st.forcedMark, 0, "There should zero marked packets due hardmark mark");
  NS_TEST_EXPECT_MSG_EQ (st.unforcedDrop, 0, "There should zero dropped packets due probability mark");
  NS_TEST_EXPECT_MSG_EQ (st.forcedDrop, 0, "There should zero dropped packets due hardmark mark");
  NS_TEST_EXPECT_MSG_EQ (st.qLimDrop, 0, "There should zero dropped packets due queue full");


  // save number of drops from tests
  struct d
  {
    uint32_t test4;
    uint32_t test5;
    uint32_t test6;
  } drop,mark;


  //test 4: more data which are ECN capable resulting in forced and unforced marks but no forced and unforced drops
  queue = CreateObject<RedQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MinTh", DoubleValue (minTh)), true,
                         "Verify that we can actually set the attribute MinTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MaxTh", DoubleValue (maxTh)), true,
                         "Verify that we can actually set the attribute MaxTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QW", DoubleValue (0.020)), true,
                         "Verify that we can actually set the attribute QW");
  queue->Initialize ();
  Enqueue (queue, pktSize, 300);
  st = queue->GetStats ();
  drop.test4 = st.unforcedDrop + st.forcedDrop - st.qLimDrop;
  mark.test4 = st.unforcedMark + st.forcedMark;

  if (queue->GetMode () == Queue::QUEUE_MODE_PACKETS)
    {
      NS_TEST_EXPECT_MSG_EQ (drop.test4, 0, "There should be no unforced or forced dropped packets");
      NS_TEST_EXPECT_MSG_EQ (mark.test4, 27, "There should be 27 marked packets with this seed, run number, and stream");
    }
  else
    {
      NS_TEST_EXPECT_MSG_EQ (drop.test4, 0, "There should be no unforced or forced dropped packets");
      NS_TEST_EXPECT_MSG_EQ (mark.test4, 55, "There should be 55 marked packets with this seed, run number, and stream");
    }

  //test 5:  more data which are not ECN capable resulting in forced and unforced drops but no forced and unforced marks
  queue = CreateObject<RedQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MinTh", DoubleValue (minTh)), true,
                         "Verify that we can actually set the attribute MinTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MaxTh", DoubleValue (maxTh)), true,
                         "Verify that we can actually set the attribute MaxTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QW", DoubleValue (0.020)), true,
                         "Verify that we can actually set the attribute QW");
  queue->Initialize ();
  EnqueueNonEcnCapable (queue, pktSize, 300);
  st = queue->GetStats ();
  drop.test5 = st.unforcedDrop + st.forcedDrop - st.qLimDrop;
  mark.test5 = st.unforcedMark + st.forcedMark;

  if (queue->GetMode () == Queue::QUEUE_MODE_PACKETS)
    {
      NS_TEST_EXPECT_MSG_EQ (mark.test5, 0, "There should be no unforced or forced marked packets");
      NS_TEST_EXPECT_MSG_EQ (drop.test5, 26, "There should be 26 unforced or forced dropped packets with this seed, run number, and stream");
    }
  else
    {
      NS_TEST_EXPECT_MSG_EQ (mark.test5, 0, "There should be no unforced or forced marked packets");
      NS_TEST_EXPECT_MSG_EQ (drop.test5, 45, "There should be 45 unforced or forced dropped packets with this seed, run number, and stream");
    }

  // test 6: There can be qlim drops with forced or unforced marks but no forced or unforced drops when packets are ECN capable
  maxTh = 100 * modeSize;
  queue = CreateObject<RedQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MinTh", DoubleValue (minTh)), true,
                         "Verify that we can actually set the attribute MinTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("MaxTh", DoubleValue (maxTh)), true,
                         "Verify that we can actually set the attribute MaxTh");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QW", DoubleValue (0.020)), true,
                         "Verify that we can actually set the attribute QW");
  queue->Initialize ();
  Enqueue (queue, pktSize, 500);
  st = queue->GetStats ();
  drop.test6 = st.unforcedDrop + st.forcedDrop - st.qLimDrop;
  mark.test6 = st.unforcedMark + st.forcedMark;
  uint32_t qLimdrop = st.qLimDrop;

  if (queue->GetMode () == Queue::QUEUE_MODE_PACKETS)
    {
      NS_TEST_EXPECT_MSG_EQ (drop.test6, 0, "There should be no unforced or forced dropped packets");
      NS_TEST_EXPECT_MSG_EQ (mark.test6, 86, "There should be 86 marked packets  due to probability mark and hard mark with this seed, run number, and stream");
      NS_TEST_EXPECT_MSG_EQ (qLimdrop, 200, "There should be 200 drops due to queue limit with this seed, run number, and stream");
    }
  else
    {
      NS_TEST_EXPECT_MSG_EQ (drop.test6, 0, "There should be no unforced or forced dropped packets");
      NS_TEST_EXPECT_MSG_NE (mark.test6, 121, "There should be 121 marked packets  due to probability mark and hard mark with this seed, run   number, and stream");
      NS_TEST_EXPECT_MSG_EQ (qLimdrop, 200, "There should be 200 drops due to queue limit with this seed, run number, and stream");
    }
}

void
EcnIpv6RedQueueDiscTestCase::Enqueue (Ptr<RedQueueDisc> queue, uint32_t size, uint32_t nPkt)
{
  Address dest;
  Ipv6Header hdr;
  hdr.SetEcn (Ipv6Header::ECN_ECT0);
  Ptr<QueueDiscItem> item;

  for (uint32_t i = 0; i < nPkt; i++)
    {
      Ptr<Packet> p = Create<Packet> (size);
      item = Create<EcnIpv6RedQueueDiscTestItem> (p, dest, 0, hdr);
      queue->Enqueue (item);
    }
}

void
EcnIpv6RedQueueDiscTestCase::EnqueueNonEcnCapable (Ptr<RedQueueDisc> queue, uint32_t size, uint32_t nPkt)
{
  Address dest;
  Ipv6Header hdr;
  Ptr<QueueDiscItem> item;

  for (uint32_t i = 0; i < nPkt; i++)
    {
      Ptr<Packet> p = Create<Packet> (size);
      item = Create<EcnIpv6RedQueueDiscTestItem> (p, dest, 0, hdr);
      queue->Enqueue (item);
    }
}

void
EcnIpv6RedQueueDiscTestCase::DoRun (void)
{
  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (17);
  RunRedTest (StringValue ("QUEUE_MODE_PACKETS"));
  RunRedTest (StringValue ("QUEUE_MODE_BYTES"));
  Simulator::Destroy ();

}

static class EcnIpv6RedQueueDiscTestSuite : public TestSuite
{
public:
  EcnIpv6RedQueueDiscTestSuite ()
    : TestSuite ("ecn-ipv6-red-queue-disc", UNIT)
  {
    AddTestCase (new EcnIpv6RedQueueDiscTestCase (), TestCase::QUICK);
  }
} g_redQueueTestSuite;
