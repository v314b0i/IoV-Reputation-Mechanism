//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/application/traci/MyVeinsNodeApp2.h"
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
using namespace veins;

Define_Module(veins::MyVeinsNodeApp2);
int MyVeinsNodeApp2::node0id = 100; // for naming vehicles correctly.
int MyVeinsNodeApp2::sentRprtGlobal;
int MyVeinsNodeApp2::sentMsgGlobal;
cOutVector* MyVeinsNodeApp2::sentRprtGlobalVector;
cOutVector* MyVeinsNodeApp2::sentMsgGlobalVector;
bool MyVeinsNodeApp2::inaccurateBoolCheck(bool val, float accuracy) {
	srand((int) simTime().raw() + myId + (int) val + (int) (accuracy * 100));
	return ((rand() % 1000) < (accuracy * 1000)) ? val : !val;
}

void MyVeinsNodeApp2::initialize(int stage) {
	DemoBaseApplLayer::initialize(stage);
	if (stage == 0) {

		EV << "Initializing " << par("appName").stringValue() << std::endl;
		lastRSUBroadcastId = -1;
		logSplitFactor = par("logSplitFactor").intValue();
		logSplitLevel = par("logSplitLevel").intValue();
		logSplitSmallest = par("logSplitSmallest").intValue();
		logSplitSizes = calculatePowersAscending<std::vector<int>>(logSplitSmallest, logSplitFactor, logSplitLevel);
		withoutReportDumpSharing = par("withoutReportDumpSharing").boolValue();
		messageInterval = par("messageInterval");
		messageIntervalVarianceLimit = par("messageIntervalVarianceLimit");
		reportGenTime = par("reportGenTime");
		reportGenTimeVarianceLimit = par("reportGenTimeVarianceLimit");
		requestDelay = par("requestDelay");
		requestDelayVarianceLimit = par("requestDelayVarianceLimit");
		requestResponseDelay = par("requestResponseDelay");

		requestResponseDelayVarianceLimit = par("requestResponseDelayVarianceLimit");
		messageIntervalVarianceLimit = SimTime(messageIntervalVarianceLimit.inUnit(SIMTIME_S), SIMTIME_MS);
		reportGenTimeVarianceLimit = SimTime(reportGenTimeVarianceLimit.inUnit(SIMTIME_S), SIMTIME_MS);
		requestDelayVarianceLimit = SimTime(requestDelayVarianceLimit.inUnit(SIMTIME_S), SIMTIME_MS);
		requestResponseDelayVarianceLimit = SimTime(requestResponseDelayVarianceLimit.inUnit(SIMTIME_S), SIMTIME_MS);
		evaluatingAccuracy = setEvaluatingAccuracy();
		sendingAccuracy = setSendingAccuracy();

		evaluatableMessages = (float) par("percentageOfInfoEvaluatable").intValue() / (float) 100;
		sendMsgEvt = new cMessage("Send Message Event", SEND_INFOMSG_EVT);
		//STATS
		sent = 0;
		sentCorrect = 0;
		recMsg = 0;
		recRprt = 0;
		sentRprt = 0;
		sentCorrectRprt = 0;
		sentDumpRequests = 0;
		sentDumps = 0;
		receivedResponseDumps = 0;
		receivedDumpRequests = 0;
		receivedDumps = 0;
		receivedRSUBroadcasts = 0;
		sentVector.setName("sentVector");
		sentCorrectVector.setName("sentCorrectVector");
		sentReportsVector.setName("sentReportsVector");
		sentCorrectReportsVector.setName("sentCorrectReportsVector");
		sentDumpsVector.setName("sentDumpsVector");
		sentDumpRequestsVector.setName("sentDumpRequestsVector");
		receivedResponseDumpsVector.setName("receivedResponseDumpsVector");
		receivedDumpsVector.setName("receivedDumpsVector");
		receivedDumpRequestsVector.setName("receivedDumpRequestsVector");
		myAccuracyVector.setName("myAccuracyVector");
		receivedRSUBroadcastsVector.setName("receivedRSUBroadcastsVector");
	} else if (stage == 1) {
		if (par("isNode0").boolValue()){
			node0id = myId;
			sentRprtGlobal=0;
			sentRprtGlobalVector=new cOutVector("sentRprtGlobalVector");
			sentMsgGlobal=0;
			sentMsgGlobalVector=new cOutVector("sentMsgGlobalVector");
		}
		//manually setting nodes 1,3 as bad senders and 0,2 as good senders.
		if ((int) myId == num2id(1, node0id) || (int) myId == num2id(3, node0id)) {
			sendingAccuracy = (float) par("badSendingAccuracyPercentage").intValue() / (float) 100;
		} else if ((int) myId == num2id(2, node0id) || (int) myId == num2id(4, node0id)) {
			sendingAccuracy = (float) par("goodSendingAccuracyPercentage").intValue() / (float) 100;
		}

		//manually setting nodes 0,1 as bad reporters
		if ((int) myId == num2id(1, node0id) || (int) myId == num2id(0, node0id))
			evaluatingAccuracy = (float) par("badEvaluatingAccuracyPercentage").intValue() / (float) 100;

		//removing the message evaluatability limitation for rogue reporting nodes
		if (evaluatingAccuracy == ((float) par("badEvaluatingAccuracyPercentage").intValue() / (float) 100))
			evaluatableMessages = 1;
		myMessagesHistory.setName(std::string("NodeVals/").append(std::to_string(id2num(myId, node0id))));
		//myMessagesHistory.enable();
		srand(myId);
		simtime_t variance = messageIntervalVarianceLimit * ((((float) (rand() % 100000)) / (float) 50000) - 1); //variance=(-limit to +limit)uniformly
		scheduleAt(simTime() + messageInterval + variance, sendMsgEvt);

	}
}

float MyVeinsNodeApp2::setSendingAccuracy() {
	srand(myId + (int) simTime().raw() + 13);
	int a = (rand() % 1000);
	return (float) (
			a < (par("percentageWithBadSendingAccuracy").intValue() * 10) ?
					par("badSendingAccuracyPercentage").intValue() : par("goodSendingAccuracyPercentage").intValue())
			/ (float) 100;
}
float MyVeinsNodeApp2::setEvaluatingAccuracy() {
	srand(myId + (int) simTime().raw() + 37);
	int a = (rand() % 1000);
	return (float) (
			a < (par("percentageWithBadEvaluatingAccuracy").intValue() * 10) ?
					par("badEvaluatingAccuracyPercentage").intValue() :
					par("goodEvaluatingAccuracyPercentage").intValue()) / (float) 100;
}

void MyVeinsNodeApp2::finish() {
	recordScalar("#sent", sent);
	recordScalar("#sentReports", sentRprt);
	recordScalar("#recievedMessages", recMsg);
	recordScalar("#recievedReports", recRprt);
	recordScalar("mySendingAccuracy", sendingAccuracy);
	recordScalar("myFinalRealAccuracy", (float) sentCorrect / (float) sent); //if set sending accuracy is not 0or1 the actual accuracy will wary while #sent is not large.
	recordScalar("myFianlRealReportingAccuracy", (float) sentCorrectRprt / (float) sentRprt);
	std::ofstream fout(std::string("R/").append(std::to_string(par("scnId").intValue())).append("/NodeFinals/NodeAcc/").append(std::to_string(id2num(myId, node0id))).c_str(),
			std::ios::out);
	fout << (float) sentCorrect / (float) sent;
	fout.close();
	std::ofstream fou(std::string("R/").append(std::to_string(par("scnId").intValue())).append("/NodeFinals/NodeRepAcc/").append(std::to_string(id2num(myId, node0id))).c_str(),
			std::ios::out);
	fou << (float) sentCorrectRprt / (float) sentRprt;
	fou.close();
	for (auto it = repScoreVector.begin(); it != repScoreVector.end(); ++it)
		for (auto a = it->second.begin(); a != it->second.end(); ++a)
			delete a->second;
	for (auto it = repScoreVector_MIN.begin(); it != repScoreVector_MIN.end(); ++it)
		delete it->second;
	for (auto it = repScoreChangeDueToBroadCast.begin(); it != repScoreChangeDueToBroadCast.end(); ++it)
		delete it->second;

	//delete sendMsgEvt;
	DemoBaseApplLayer::finish();
}

void MyVeinsNodeApp2::onWSM(BaseFrame1609_4 *frame) {
	//TODO deal with reports on self, or maybe not
	if (infoMsg *wsm = dynamic_cast<infoMsg*>(frame)) { //TODO
		recMsg++;
		int senderId = wsm->getSenderAddress();
		int msgId = wsm->getMsgId();
		recorder mvec(std::string("infoMsg:").append(std::to_string(id2num(myId, node0id))).append(".txt"));
		mvec.record(1);
		if (vehicles.find(senderId) == vehicles.end()) //if neither a message from this sender not a report on this sender has been recieved before
			initVehicle(senderId, false);
		mvec.record(2);
		vehMsgHistoryDynamic2 &veh = *(vehicles[senderId]);
		mvec.record(3);
		if (veh.lockedMaxId >= msgId) {
			mvec.deletefile();
			return;
		}
		mvec.record(4);
		veh.messagesRecv++;
		mvec.record(5);
		if (inaccurateBoolCheck(true, evaluatableMessages)) {
			mvec.recordString(std::string("6A"));
			bool val = inaccurateBoolCheck(wsm->getCorrect(), evaluatingAccuracy);
			sentReportsVector.record(++sentRprt);
			sentRprtGlobalVector->record(++sentRprtGlobal);

			std::ofstream fou(std::string("R/").append(std::to_string(par("scnId").intValue())).append("/NodeFinals/NodeRepAcc/").append(std::to_string(id2num(myId, node0id))).c_str(),
					std::ios::trunc);
			fou << (float) sentCorrectRprt / (float) sentRprt;
			//temporary fix for misc error (veins or omnetpp). some nodes weren't doing this in the finish() function.
			fou.close();
			if (val == wsm->getCorrect())
				sentCorrectReportsVector.record(++sentCorrectRprt);
			veh.insertMyReport(myId, msgId, val);
			reportMsg *rep = new reportMsg();
			populateWSM(rep);
			rep->setName("Report Message");
			rep->setReporterAddress(myId);
			rep->setReporteeAddress(senderId);
			rep->setReportedMsgId(msgId);
			rep->setFoundValid(val);
			rep->setKind(REPORT_MSG);
			srand((int) simTime().raw() + myId + msgId + senderId + 3851);
			simtime_t variance = reportGenTimeVarianceLimit * ((((float) (rand() % 100000)) / (float) 50000) - 1);
			scheduleAt(simTime() + reportGenTime + variance, rep);

			//STATS
			recordVehScores(senderId);
		} else {
			mvec.recordString(std::string("6A"));
		}
		mvec.deletefile();
	}

	else if (reportMsg *wsm = dynamic_cast<reportMsg*>(frame)) {
		recRprt++;
		recorder mvec(std::string("reportMsg:").append(std::to_string(id2num(myId, node0id))).append(".txt"));
		int reporteeId = wsm->getReporteeAddress();
		int reporterId = wsm->getReporterAddress();
		if (reporterId == reporteeId)
			return; //not going to happen but failsafe for modification in report gen block.
		int msgId = wsm->getReportedMsgId();
		bool foundValid = wsm->getFoundValid();
		if (blacklistedReporters.count(reporterId))
			return;
		mvec.record(1);
		if (!vehicles.count(reporteeId))
			initVehicle(reporteeId);
		mvec.record(2);
		vehMsgHistoryDynamic2 &veh = *(vehicles[reporteeId]);
		if (veh.lockedMaxId >= msgId)
			return;
		mvec.record(3);
		veh.reportsRecv++;
		veh.insertReport(reporterId, msgId, foundValid);
		mvec.record(4);
		recordVehScores(reporteeId);
		mvec.deletefile();
	} else if (withoutReportDumpSharing)
		return;
	else if (requestDumpMsg *wsm = dynamic_cast<requestDumpMsg*>(frame)) {
		try {
			recorder rec(std::string("requestDumpMsg:").append(std::to_string(id2num(myId, node0id))).append(".txt"));
			rec.record(0);
			receivedDumpRequestsVector.record(++receivedDumpRequests);
			int senderId = wsm->getSenderAddress();
			int requestedReporteeId = wsm->getRequestedReporteeAddress();
			if (vehicles.find(requestedReporteeId) == vehicles.end()) {
				rec.deletefile();
				return;
			}
			vehMsgHistoryDynamic2 &veh = *(vehicles[requestedReporteeId]);
			std::string trueMsgs = "", falseMsgs = "", mId;
			for (auto msg : veh.myReports) {
				mId = std::to_string(msg.first).append(",");
				if (msg.second)
					trueMsgs.append(mId);
				else falseMsgs.append(mId);
			}
			if (trueMsgs.length() > 2 || falseMsgs.length() > 2) { //TODO more defined cutoff
				reportDumpMsg *dump = new reportDumpMsg();
				populateWSM(dump);
				dump->setName("Report Dump Message");
				dump->setSenderAddress(myId);
				dump->setReporteeAddress(requestedReporteeId);
				dump->setPrimaryRecipientAddress(senderId); // for stats collection at "sender"'s side on how many own requests were obliged.
				dump->setTrueMsgs(trueMsgs.c_str());
				dump->setFalseMsgs(falseMsgs.c_str());
				srand((int) simTime().raw() + myId + requestedReporteeId + senderId + 108);
				simtime_t variance = requestResponseDelayVarianceLimit
						* ((((float) (rand() % 100000)) / (float) 50000) - 1);
				scheduleAt(simTime() + requestResponseDelay + variance, dump);
			}
			rec.deletefile();
		} catch (...) {
			recordScalar("onWSM:requestDumpMsg threw error", 1);
		}
	} else if (reportDumpMsg *wsm = dynamic_cast<reportDumpMsg*>(frame)) {
		try {
			recorder rec(std::string("reportDumpMsg:").append(std::to_string(id2num(myId, node0id))).append(".txt"));
			rec.record(0);
			receivedDumpsVector.record(++receivedDumps);
			if ((int) wsm->getPrimaryRecipientAddress() == myId)
				receivedResponseDumpsVector.record(++receivedResponseDumps);
			int reporteeId = wsm->getReporteeAddress();
			if (requestDumpMsgPointers.count(reporteeId)) {
				rec.record(0.5);
				if (requestDumpMsgPointers[reporteeId]->isScheduled())
					//cancelEvent(requestDumpMsgPointers[reporteeId]);
					requestDumpMsgPointers.erase(reporteeId);
			}
			rec.record(1);
			int senderId = wsm->getSenderAddress();
			if (senderId == reporteeId)
				return;
			//TODO should i init sender too?
			if (vehicles.find(reporteeId) == vehicles.end())
				initVehicle(reporteeId, true);
			intSet trueMsgs = csvToIntSet(std::string(wsm->getTrueMsgs()));
			intSet falseMsgs = csvToIntSet(std::string(wsm->getFalseMsgs()));
			rec.record(2);
			vehMsgHistoryDynamic2 &veh = *(vehicles[reporteeId]);
			for (auto msgId : trueMsgs)
				veh.insertReport(senderId, msgId, true);
			for (auto msgId : falseMsgs)
				veh.insertReport(senderId, msgId, false);
			rec.deletefile();
		} catch (...) {
			recordScalar("onWSM:reportDumpMsg threw error", 1);
		}
	} else if (RSUBroadcast *wsm = dynamic_cast<RSUBroadcast*>(frame)) {
		try {
			recorder rec(
					std::string("wsm=RSUBroadcast").append(std::to_string(myId)).append(
							std::to_string(wsm->getBroadcastId())).append(".txt"));
			rec.record(wsm->getBroadcastId());
			receivedRSUBroadcastsVector.record(++receivedRSUBroadcasts);
			if (lastRSUBroadcastId < wsm->getBroadcastId()) {
				rec.record(66);
				blacklistedReporters = csvToIntSet(std::string(wsm->getBlacklistCSV()));
				ingestRSUBroadcast(std::string(wsm->getVehIdAndScoresCSV()));
				rec.record(1);
				rec.recordString("\n---\n");
				rec.recordString(std::string(wsm->getVehIdAndScoresCSV()));
				lastRSUBroadcastId = wsm->getBroadcastId();
			}
			//rec.deletefile();
		} catch (...) {
			recordScalar("onWSM:RSUBroadcast threw error", 1);
		}
	}
}
void MyVeinsNodeApp2::ingestRSUBroadcast(std::string csvStr) {
	recorder rec(std::string("ingestRSUBroadcast").append(std::to_string(myId)).append(".txt"));
	rec.recordString(csvStr);
	rec.recordString("\n-----\n");
	int_2_floatVec reportsCsv2DFloatValsParse = csv2DFloatValsParse(csvStr, logSplitSizes.size() + 2);
	rec.record(1);
	for (auto i : reportsCsv2DFloatValsParse) {
		rec.recordString(std::string("\ninloop ").append(std::to_string(i.first)));
		float splitavg0;
		if (vehicles.count(i.first) == 0) {
			initVehicle(i.first);
			splitavg0 = 0;
		} else splitavg0 = vehicles[i.first]->getOverallAvg();
		vehicles[i.first]->ingestRSUScore(i.second, blacklistedReporters);
		float splitavg1 = vehicles[i.first]->getOverallAvg();
		repScoreChangeDueToBroadCast[i.first]->record(splitavg1 - splitavg0);
	}
	rec.deletefile();
}
void MyVeinsNodeApp2::initVehicle(int id, bool dontRequestDump) {
	try {
		vehicles[id] = new vehMsgHistoryDynamic2(logSplitSmallest, logSplitFactor, logSplitLevel,
				par("stagingMessagesEnabled").boolValue());
		repScoreVector[id] = std::tr1::unordered_map<int, cOutVector*>();
		for (auto size : logSplitSizes) {
			repScoreVector[id][size] = new cOutVector(
					std::string("RepScoreVector-SPLIT").append(std::to_string(size)).append(" - ").append(
							std::to_string(id2num(id, node0id))).c_str());
		}
		repScoreVector_MIN[id] = new cOutVector(
				std::string("RepScoreVector-MIN").append(" - ").append(std::to_string(id2num(id, node0id))).c_str());
		repScoreChangeDueToBroadCast[id] =
				new cOutVector(
						std::string("repScoreChangeDueToBroadCast").append(" - ").append(
								std::to_string(id2num(id, node0id))).c_str());
		if (!(dontRequestDump || withoutReportDumpSharing || requestDumpMsgPointers.count(id))) {
			requestDumpMsg *req = new requestDumpMsg();
			populateWSM(req);
			req->setName("Dump Request Message");
			req->setSenderAddress(myId);
			req->setRequestedReporteeAddress(id);
			srand((int) simTime().raw() + myId + id + sent + 667);
			simtime_t variance = requestDelayVarianceLimit * ((((float) (rand() % 100000)) / (float) 50000) - 1);
			scheduleAt(simTime() + requestDelay + variance, req);
			requestDumpMsgPointers[id] = req;
		}
	} catch (...) {
		recordScalar("initVehicle threw error", 1);
	}
}
void MyVeinsNodeApp2::recordVehScores(int id) {
	auto splitavgs = vehicles[id]->getSplitAvgs();
	for (auto score : splitavgs)
		repScoreVector[id][score.first]->record(score.second);
	float min = vehicles[id]->getMinAvg();
	if (min != 2)
		repScoreVector_MIN[id]->record(min);
}
void MyVeinsNodeApp2::handleSelfMsg(cMessage *msg) {
	try {
		if (infoMsg *wsm = dynamic_cast<infoMsg*>(msg)) {
			if (wsm->getCorrect())
				sentCorrectVector.record(sentCorrect);
			sentVector.record(sent);
			sendDown(wsm);
		} else if (reportMsg *wsm = dynamic_cast<reportMsg*>(msg)) {
			sendDown(wsm);
		} else if (requestDumpMsg *wsm = dynamic_cast<requestDumpMsg*>(msg)) {
			sentDumpRequestsVector.record(++sentDumpRequests);
			sendDown(wsm);
		} else if (reportDumpMsg *wsm = dynamic_cast<reportDumpMsg*>(msg)) {
			sentDumpsVector.record(++sentDumps);
			sendDown(wsm);
		} else if (msg->getKind() == SEND_INFOMSG_EVT) {
			infoMsg *wsm = new infoMsg();
			populateWSM(wsm);
			wsm->setName("Info Message");
			wsm->setSenderAddress(myId);
			wsm->setMsgId(sent++);
			wsm->setCorrect(inaccurateBoolCheck(true, sendingAccuracy));
			wsm->setKind(INFO_MSG);
			sendDown(wsm);
			//STATS
			if (wsm->getCorrect())
				sentCorrectVector.record(++sentCorrect);
			sentVector.record(sent);
			sentMsgGlobalVector->record(++sentMsgGlobal);
			myAccuracyVector.record((float) sentCorrect / (float) sent);
			myMessagesHistory.recordString(
					std::string("\n").append(std::to_string(sent-1)).append(",").append(
							std::to_string(wsm->getCorrect() ? 1 : 0)));
			//-----
			srand((int) simTime().raw() + myId + sent + 5643);
			simtime_t variance = messageIntervalVarianceLimit * ((((float) (rand() % 100000)) / (float) 50000) - 1);
			scheduleAt(simTime() + messageInterval + variance, sendMsgEvt);
		} else DemoBaseApplLayer::handleSelfMsg(msg);
	} catch (...) {
		recordScalar("handleSelfMsg threw error", 1);
	}
}

void MyVeinsNodeApp2::handlePositionUpdate(cObject *obj) {
	DemoBaseApplLayer::handlePositionUpdate(obj);
}

