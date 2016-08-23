/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright © 2011 Marcos Talau
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
 * Author: Marcos Talau (talau@users.sourceforge.net)
 * Modified by:   Pasquale Imputato <p.imputato@gmail.com>
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
#include "ns3/ipv4-queue-disc-item.h"

using namespace ns3;

class EcnRedQueueDiscTestItem : public Ipv4QueueDiscItem {
public:
  EcnRedQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol, const Ipv4Header & header);
  virtual ~EcnRedQueueDiscTestItem ();

private:
  EcnRedQueueDiscTestItem ();
  EcnRedQueueDiscTestItem (const EcnRedQueueDiscTestItem &);
  EcnRedQueueDiscTestItem &operator = (const EcnRedQueueDiscTestItem &);
};

EcnRedQueueDiscTestItem::EcnRedQueueDiscTestItem (Ptr<Packet> p, const Address & addr, uint16_t protocol, const Ipv4Header & header)
  : Ipv4QueueDiscItem (p, addr, protocol, header)
{
}

EcnRedQueueDiscTestItem::~EcnRedQueueDiscTestItem ()
{
}

class EcnRedQueueDiscTestCase : public TestCase
{
public:
  EcnRedQueueDiscTestCase ();
  virtual void DoRun (void);
private:
  void Enqueue (Ptr<RedQueueDisc> queue, uint32_t size, uint32_t nPkt);
  void RunRedTest (StringValue mode);
};

EcnRedQueueDiscTestCase::EcnRedQueueDiscTestCase ()
  : TestCase ("Sanity check on the red queue implementation")
{
}

void
EcnRedQueueDiscTestCase::RunRedTest (StringValue mode)
{
  uint32_t pktSize = 0;
  // 1 for packets; pktSize for bytes
  uint32_t modeSize = 1;
  double minTh = 2;
  double maxTh = 5;
  uint32_t qSize = 8;
  Ptr<RedQueueDisc> queue = CreateObject<RedQueueDisc> ();

  // test 1: simple enqueue/dequeue with no drops
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
      modeSize = pktSize + 20;
      queue->SetTh (minTh * modeSize, maxTh * modeSize);
      queue->SetQueueLimit (qSize * modeSize);
    }

  Ipv4Header hdr;
  hdr.SetEcn(Ipv4Header::ECN_ECT0);
  
  Ptr<Packet> p1, p2, p3, p4, p5, p6, p7, p8;
  p1 = Create<Packet> (pktSize);
 
  p2 = Create<Packet> (pktSize);
 
  p3 = Create<Packet> (pktSize);
 
  p4 = Create<Packet> (pktSize);
 
  p5 = Create<Packet> (pktSize);
 
  p6 = Create<Packet> (pktSize);
  
  p7 = Create<Packet> (pktSize);
  
  p8 = Create<Packet> (pktSize);
 
  queue->Initialize ();
  
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 0 * modeSize, "There should be no packets in there");
  queue->Enqueue (Create<EcnRedQueueDiscTestItem> (p1, dest, 0, hdr));
  
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 1 * modeSize, "There should be one packet in there");
  queue->Enqueue (Create<EcnRedQueueDiscTestItem> (p2, dest, 0, hdr));
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 2 * modeSize, "There should be two packets in there");
  queue->Enqueue (Create<EcnRedQueueDiscTestItem> (p3, dest, 0, hdr));
  queue->Enqueue (Create<EcnRedQueueDiscTestItem> (p4, dest, 0, hdr));
  queue->Enqueue (Create<EcnRedQueueDiscTestItem> (p5, dest, 0, hdr));
  queue->Enqueue (Create<EcnRedQueueDiscTestItem> (p6, dest, 0, hdr));
  queue->Enqueue (Create<EcnRedQueueDiscTestItem> (p7, dest, 0, hdr));
  queue->Enqueue (Create<EcnRedQueueDiscTestItem> (p8, dest, 0, hdr));
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 8 * modeSize, "There should be eight packets in there");

  
  Ptr<Ipv4QueueDiscItem> item;

  item = StaticCast<Ipv4QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the first packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 7 * modeSize, "There should be seven packets in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p1->GetUid (), "was this the first packet ?");
     
  item = StaticCast<Ipv4QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the second packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 6 * modeSize, "There should be six packet in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p2->GetUid (), "Was this the second packet ?");
 
  item = StaticCast<Ipv4QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the third packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 5 * modeSize, "There should be five packets in there");
  NS_TEST_EXPECT_MSG_EQ (item->GetPacket ()->GetUid (), p3->GetUid (), "Was this the third packet ?");

  item = StaticCast<Ipv4QueueDiscItem> (queue->Dequeue ());
  item = StaticCast<Ipv4QueueDiscItem> (queue->Dequeue ());
  item = StaticCast<Ipv4QueueDiscItem> (queue->Dequeue ());
  item = StaticCast<Ipv4QueueDiscItem> (queue->Dequeue ());
  item = StaticCast<Ipv4QueueDiscItem> (queue->Dequeue ());

  item = StaticCast<Ipv4QueueDiscItem> (queue->Dequeue ());
  NS_TEST_EXPECT_MSG_EQ ((item == 0), true, "There are really no packets in there");

 
  // test 2: more data, but with no marks or drops
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
  Enqueue (queue, pktSize, 200);
  RedQueueDisc::Stats st = StaticCast<RedQueueDisc> (queue)->GetStats ();
  NS_TEST_EXPECT_MSG_EQ (st.unforcedMark, 0, "There should zero marked packets due probability mark");
  NS_TEST_EXPECT_MSG_EQ (st.forcedMark, 0, "There should zero marked packets due hardmark mark");
  NS_TEST_EXPECT_MSG_EQ (st.unforcedDrop, 0, "There should zero dropped packets due probability mark");
  NS_TEST_EXPECT_MSG_EQ (st.forcedDrop, 0, "There should zero dropped packets due hardmark mark");
  NS_TEST_EXPECT_MSG_EQ (st.qLimDrop, 0, "There should zero dropped packets due queue full");

  
  // save number of drops from tests
  struct d {
    uint32_t test3;
    uint32_t test4;
  } drop,mark;
 
  
  // test 3: more data, now drops due QW change
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
  st = StaticCast<RedQueueDisc> (queue)->GetStats ();
  drop.test3 = st.unforcedDrop + st.forcedDrop - st.qLimDrop;
  mark.test3 = st.unforcedMark + st.forcedMark;
  NS_TEST_EXPECT_MSG_EQ (drop.test3, 0, "There should be no unforced or forced dropped packets");
  NS_TEST_EXPECT_MSG_NE (mark.test3, 0, "There should be some marked packets");
  
   
  // test 4: reduced maxTh, this causes more drops
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
  st = StaticCast<RedQueueDisc> (queue)->GetStats ();
  drop.test4 = st.qLimDrop;
  NS_TEST_EXPECT_MSG_NE (drop.test4, 0, "There should be some dropped packets due to Queuelimit");
  uint32_t drops = st.unforcedDrop + st.forcedDrop - st.qLimDrop;
  NS_TEST_EXPECT_MSG_EQ (drops, 0, "There should be no unforced and forced dropped packets");
  mark.test4 = st.unforcedMark + st.forcedMark; 
  NS_TEST_EXPECT_MSG_NE (mark.test4, 0, "There should be some marked packets  due to probability mark and hard mark");
 
 }

void 
EcnRedQueueDiscTestCase::Enqueue (Ptr<RedQueueDisc> queue, uint32_t size, uint32_t nPkt)
{
  Address dest;
  Ipv4Header hdr;
  hdr.SetEcn(Ipv4Header::ECN_ECT0);

  for (uint32_t i = 0; i < nPkt; i++)
    {
      Ptr<Packet> p = Create<Packet> (size);
      p->AddHeader(hdr);
      queue->Enqueue (Create<EcnRedQueueDiscTestItem> (p, dest, 0, hdr));
    }
}

void
EcnRedQueueDiscTestCase::DoRun (void)
{
  RunRedTest (StringValue ("QUEUE_MODE_PACKETS"));
  RunRedTest (StringValue ("QUEUE_MODE_BYTES"));
  Simulator::Destroy ();

}

static class EcnRedQueueDiscTestSuite : public TestSuite
{
public:
  EcnRedQueueDiscTestSuite ()
    : TestSuite ("ecn-red-queue-disc", UNIT)
  {
    AddTestCase (new EcnRedQueueDiscTestCase (), TestCase::QUICK);
  }
} g_redQueueTestSuite;
