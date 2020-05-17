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

#include "veins/modules/application/traci/MyVeinsNodeStub.h"
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
using namespace veins;

Define_Module(veins::MyVeinsNodeStub);
int MyVeinsNodeStub::node0id = 100; // for naming vehicles correctly.
int MyVeinsNodeStub::sentRprtGlobal;
int MyVeinsNodeStub::sentMsgGlobal;
cOutVector *MyVeinsNodeStub::sentRprtGlobalVector;
cOutVector *MyVeinsNodeStub::sentMsgGlobalVector;
intSet MyVeinsNodeStub::collusionTargets;

bool MyVeinsNodeStub::inaccurateBoolCheck(bool val, float accuracy) {
	srand((int) simTime().raw() + myId + (int) val + (int) (accuracy * 100));
	return ((rand() % 1000) < (accuracy * 1000)) ? val : !val;
}

void MyVeinsNodeStub::initialize(int stage) {
	DemoBaseApplLayer::initialize(stage);
	if (stage == 0) {
		enableStats = false;
		EV << "Initializing " << par("appName").stringValue() << std::endl;
		lastRSUBroadcastId = -1;
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
		if (enableStats) {
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
		}
	} else if (stage == 1) {
		if (par("isNode0").boolValue()) {
			node0id = myId;
			sentRprtGlobal = 0;
			if (enableStats)
				sentRprtGlobalVector = new cOutVector("sentRprtGlobalVector");
			sentMsgGlobal = 0;
			if (enableStats)
				sentMsgGlobalVector = new cOutVector("sentMsgGlobalVector");
		}
		/*//manually setting nodes 1,3 as bad senders and 0,2 as good senders.
		 if ((int) myId == num2id(1, node0id) || (int) myId == num2id(3, node0id)) {
		 sendingAccuracy = (float) par("badSendingAccuracyPercentage").intValue() / (float) 100;
		 } else if ((int) myId == num2id(2, node0id) || (int) myId == num2id(4, node0id)) {
		 sendingAccuracy = (float) par("goodSendingAccuracyPercentage").intValue() / (float) 100;
		 }

		 //manually setting nodes 0,1 as bad reporters
		 if ((int) myId == num2id(1, node0id) || (int) myId == num2id(0, node0id))
		 evaluatingAccuracy = (float) par("badEvaluatingAccuracyPercentage").intValue() / (float) 100;
		 */
		//removing the message evaluatability limitation for rogue reporting nodes
		if (evaluatingAccuracy == ((float) par("badEvaluatingAccuracyPercentage").intValue() / (float) 100))
			evaluatableMessages = 1;

		isColluding = inaccurateBoolCheck(true, ((float) par("colludingNodesPercent").intValue()) / (float) 100);
		if (!isColluding) {
			float fractionOfTargetsAmongNonColluders = ((float) par("collusionTargetsPercent").intValue())
					/ (float) (100 - par("colludingNodesPercent").intValue());
			if (inaccurateBoolCheck(true, fractionOfTargetsAmongNonColluders))
				collusionTargets.insert(myId);
		}

		srand(myId);
		simtime_t variance = messageIntervalVarianceLimit * ((((float) (rand() % 100000)) / (float) 50000) - 1); //variance=(-limit to +limit)uniformly
		scheduleAt(simTime() + messageInterval + variance, sendMsgEvt);

	}
}

float MyVeinsNodeStub::setSendingAccuracy() {
	srand(myId + (int) simTime().raw() + 13);
	int a = (rand() % 1000);
	return (float) (
			a < (par("percentageWithBadSendingAccuracy").intValue() * 10) ?
					par("badSendingAccuracyPercentage").intValue() : par("goodSendingAccuracyPercentage").intValue())
			/ (float) 100;
}
float MyVeinsNodeStub::setEvaluatingAccuracy() {
	srand(myId + (int) simTime().raw() + 37);
	int a = (rand() % 1000);
	return (float) (
			a < (par("percentageWithBadEvaluatingAccuracy").intValue() * 10) ?
					par("badEvaluatingAccuracyPercentage").intValue() :
					par("goodEvaluatingAccuracyPercentage").intValue()) / (float) 100;
}

void MyVeinsNodeStub::finish() {
	recordScalar("#sent", sent);
	recordScalar("#sentReports", sentRprt);
	recordScalar("#recievedMessages", recMsg);
	recordScalar("#recievedReports", recRprt);
	recordScalar("mySendingAccuracy", sendingAccuracy);
	recordScalar("myFinalRealAccuracy", (float) sentCorrect / (float) sent); //if set sending accuracy is not 0or1 the actual accuracy will wary while #sent is not large.
	recordScalar("myFianlRealReportingAccuracy", (float) sentCorrectRprt / (float) sentRprt);
	std::ofstream fout(
			std::string("R/").append(std::to_string(par("scnId").intValue())).append("/NodeFinals/NodeAcc/").append(
					std::to_string(id2num(myId, node0id))).c_str(), std::ios::out);
	fout << (float) sentCorrect / (float) sent;
	fout.close();
	std::ofstream fou(
			std::string("R/").append(std::to_string(par("scnId").intValue())).append("/NodeFinals/NodeRepAcc/").append(
					std::to_string(id2num(myId, node0id))).c_str(), std::ios::out);
	fou << (float) sentCorrectRprt / (float) sentRprt;
	fou.close();
	//delete sendMsgEvt;
	DemoBaseApplLayer::finish();
}

void MyVeinsNodeStub::onWSM(BaseFrame1609_4 *frame) {
	//TODO deal with reports on self, or maybe not
	if (infoMsg *wsm = dynamic_cast<infoMsg*>(frame)) { //TODO
		recMsg++;
		int senderId = wsm->getSenderAddress();
		int msgId = wsm->getMsgId();
		if (myReports.find(senderId) == myReports.end()) //if neither a message from this sender not a report on this sender has been recieved before
			initVehicle(senderId, false);
		if (isColluding and collusionTargets.count(senderId)) {

		}
		if ((isColluding and collusionTargets.count(senderId)) or inaccurateBoolCheck(true, evaluatableMessages)) {
			bool val = inaccurateBoolCheck(wsm->getCorrect(), evaluatingAccuracy);
			++sentRprt;
			++sentRprtGlobal;
			if (isColluding and collusionTargets.count(senderId))
				val = !wsm->getCorrect();
			if (val == wsm->getCorrect())
				++sentCorrectRprt;
			if (enableStats) {
				sentCorrectReportsVector.record(sentCorrectRprt);
				sentReportsVector.record(sentRprt);
				sentRprtGlobalVector->record(sentRprtGlobal);
			}
			std::ofstream fou(
					std::string("R/").append(std::to_string(par("scnId").intValue())).append("/NodeFinals/NodeRepAcc/").append(
							std::to_string(id2num(myId, node0id))).c_str(), std::ios::trunc);
			fou << (float) sentCorrectRprt / (float) sentRprt;
			//temporary fix for misc error (veins or omnetpp). some nodes weren't doing this in the finish() function.
			fou.close();
			myReports[senderId][msgId] = val;
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
		}
	}

	else if (reportMsg *wsm = dynamic_cast<reportMsg*>(frame)) {
		recRprt++;
		int reporteeId = wsm->getReporteeAddress();
		int reporterId = wsm->getReporterAddress();
		if (reporterId == reporteeId)
			return; //not going to happen but failsafe for modification in report gen block.
		int msgId = wsm->getReportedMsgId();
		bool foundValid = wsm->getFoundValid();
		if (!myReports.count(reporteeId))
			initVehicle(reporteeId, false);
	} else if (withoutReportDumpSharing)
		return;
	else if (requestDumpMsg *wsm = dynamic_cast<requestDumpMsg*>(frame)) {
		try {
			receivedDumpRequestsVector.record(++receivedDumpRequests);
			int senderId = wsm->getSenderAddress();
			int requestedReporteeId = wsm->getRequestedReporteeAddress();
			if (myReports.find(requestedReporteeId) == myReports.end()) {
				initVehicle(requestedReporteeId, true);
				return;
			}
			std::string trueMsgs = "", falseMsgs = "", mId;
			for (auto rep : myReports[requestedReporteeId]) {
				mId = std::to_string(rep.first).append(",");
				if (rep.second)
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
		} catch (...) {
			recordScalar("onWSM:requestDumpMsg threw error", 1);
		}
	} else if (reportDumpMsg *wsm = dynamic_cast<reportDumpMsg*>(frame)) {
		try {
			receivedDumpsVector.record(++receivedDumps);
			if ((int) wsm->getPrimaryRecipientAddress() == myId)
				receivedResponseDumpsVector.record(++receivedResponseDumps);
			int reporteeId = wsm->getReporteeAddress();
			int senderId = wsm->getSenderAddress();
			if (senderId == reporteeId)
				return;
			//TODO should i init sender too?
			if (myReports.find(reporteeId) == myReports.end())
				initVehicle(reporteeId, true);
		} catch (...) {
			recordScalar("onWSM:reportDumpMsg threw error", 1);
		}
	} else if (RSUBroadcast *wsm = dynamic_cast<RSUBroadcast*>(frame)) {
		receivedRSUBroadcastsVector.record(++receivedRSUBroadcasts);
	}
}

void MyVeinsNodeStub::initVehicle(int id, bool dontRequestDump) {
	try {
		myReports[id] = std::tr1::unordered_map<int, bool>();
		if (!(dontRequestDump || withoutReportDumpSharing)) {
			requestDumpMsg *req = new requestDumpMsg();
			populateWSM(req);
			req->setName("Dump Request Message");
			req->setSenderAddress(myId);
			req->setRequestedReporteeAddress(id);
			srand((int) simTime().raw() + myId + id + sent + 667);
			simtime_t variance = requestDelayVarianceLimit * ((((float) (rand() % 100000)) / (float) 50000) - 1);
			scheduleAt(simTime() + requestDelay + variance, req);
		}
	} catch (...) {
		recordScalar("initVehicle threw error", 1);
	}
}
void MyVeinsNodeStub::handleSelfMsg(cMessage *msg) {
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
				++sentCorrect;
			++sentMsgGlobal;
			if (enableStats) {
				sentCorrectVector.record(sentCorrect);
				sentVector.record(sent);
				sentMsgGlobalVector->record(sentMsgGlobal);
				myAccuracyVector.record((float) sentCorrect / (float) sent);
			}
			//-----
			srand((int) simTime().raw() + myId + sent + 5643);
			simtime_t variance = messageIntervalVarianceLimit * ((((float) (rand() % 100000)) / (float) 50000) - 1);
			scheduleAt(simTime() + messageInterval + variance, sendMsgEvt);
		} else DemoBaseApplLayer::handleSelfMsg(msg);
	} catch (...) {
		recordScalar("handleSelfMsg threw error", 1);
	}
}

void MyVeinsNodeStub::handlePositionUpdate(cObject *obj) {
	DemoBaseApplLayer::handlePositionUpdate(obj);
}

