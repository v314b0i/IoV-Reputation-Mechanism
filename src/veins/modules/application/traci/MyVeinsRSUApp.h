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

#pragma once

#include "veins/veins.h"
#include <tr1/unordered_map>
#include <boost/functional/hash.hpp>
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/infoMsg_m.h"
#include "veins/modules/application/traci/reportMsg_m.h"

//using namespace omnetpp;

namespace veins {
typedef std::tr1::unordered_map<int, bool> reporterId_2_val;
typedef std::tr1::unordered_map<int, reporterId_2_val*> msgId_2_reporterId2val;
//typedef std::tr1::unordered_map<int, msgId_2_reporterId2val*> senderId_2_msgId2reporterId2val;
/*typedef std::tr1::unordered_map<std::pair<int, int>, int2boolmap*,
 boost::hash<std::pair<int, int>>> intpair_2_int2boolmapptr;*/

struct vehStats {
	float rep = 0.5;       		//current calculated rep of vehicle
	float repOrignal = 0.5;		//rep of vehicle from last server communication
	int reportedCount = 0; //number of reports received on this vehicle + number of reports by this vehicle that were compared with a report by self.
	int reportedTrueCount = 0; //number of reports received on this vehicle that stated it was truthful + number of reports by this vehicle that matched with a report by self.
	msgId_2_reporterId2val messages;

	//--FOR STATS--
	int reportComparisonCount = 0; //number of times a report by it was compared by a report by self and result treated as a report on it
	int reportComparisonTrueCount = 0; //number of times a report by it was compared by a report by self and it was a match.
	int msgCount = 0;      		//messages received from veh
	int trueMsgCount = 0; //messages received from veh which were evaluated to true by self
};


class VEINS_API MyVeinsRSUApp: public DemoBaseApplLayer {
public:
	void initialize(int stage) override;
	void finish() override;
	enum MyApplMessageKinds {
		SEND_INFOMSG_EVT,
		//SEND_REPORTMSG_EVT,
		INFO_MSG,
		REPORT_MSG
	};
protected:
	void onWSM(BaseFrame1609_4 *wsm) override;
	void onWSA(DemoServiceAdvertisment *wsa) override;

	void handleSelfMsg(cMessage *msg) override;
	void handlePositionUpdate(cObject *obj) override;
	bool inaccurateBoolCheck(bool val, float accuracy = 0.9); //used for simulating errors while evaluating and generating information.
	float scoreCalculator(float old, int trueCount, int count);
	float setSendingAccuracy();
	void initVehicle(int id); //to create new entry in vehicles map and stats collection maps
	int currentSubscribedServiceId;
	//storing various data on other vehicles in network map{vehId->vehStats}
	std::tr1::unordered_map<int, vehStats*> vehicles;

	// Abandoned data structs... switched to having everything in the vehicles object.
	//storing reports for each message.  map{ <senderId,msgId> --> map{ reporter->value } }
	//intpair_2_int2boolmapptr reportsArchive;
	//storing reports for each message.  "map{ <senderId> --> map{ <msgId> --> map{ reporter->value } } }"
	//senderId_2_msgId2reporterId2val reportsArchive;
	int sent;
	float sendingAccuracy;  // to control behaviour of node.
	float evaluatingAccuracy;
	cMessage *sendMsgEvt;
	//cMessage* sendReportEvt;
	simtime_t messageInterval;
	simtime_t reportDelay;
	simtime_t messageIntervalVarianceLimit;
	simtime_t reportGenTime;
	simtime_t reportGenTimeVarianceLimit;

	//--FOR STATS--
	int sentCorrect;
	int recMsg;
	int recRprt;
	int sentRprt;
	cOutVector sentVector;
	cOutVector sentCorrectVector;
	cOutVector sentReportsVector;
	std::tr1::unordered_map<int, cOutVector*> repScoreVector;
	std::tr1::unordered_map<int, cHistogram*> repScoreStats;
	std::tr1::unordered_map<int, cOutVector*> reportedVector;
	std::tr1::unordered_map<int, cOutVector*> reportedTrueVector;
	std::tr1::unordered_map<int, cOutVector*> reportComparisonVector;
	std::tr1::unordered_map<int, cOutVector*> msgVector;
};

} // namespace veins
