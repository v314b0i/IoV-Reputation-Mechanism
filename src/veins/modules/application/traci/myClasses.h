#pragma once

#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <math.h>

//TODO create initialisers to dyn alloc new objs to ptrs and remove code from various initVeh funcs
//TODO create separate structs for stats collection counters, keep app functionality and ststs collection differentiated.
enum MyApplMessageKinds {
	SEND_INFOMSG_EVT, STAGE_SHIFT_EVT, INFO_MSG, REPORT_MSG
};
typedef std::tr1::unordered_map<int, bool> int_2_bool;
typedef std::tr1::unordered_map<int, float> int_2_float;
typedef std::tr1::unordered_map<int, int> int_2_int;
typedef std::tr1::unordered_map<int, int_2_bool*> int_2_int2bool;
class vehStats {		//	{senderId : (rep, rep0, total#, true#, { msgId: { reporterId : val } }) }
public:
	float rep = 0.5;       		//current calculated rep of vehicle
	float repOrignal = 0.5;		//rep of vehicle from last server communication
	int reportedCount = 0; //number of reports received on this vehicle + number of reports by this vehicle that were compared with a report by self.
	int reportedTrueCount = 0; //number of reports received on this vehicle that stated it was truthful + number of reports by this vehicle that matched with a report by self.
	int_2_int2bool messages; // { msgId: { reporterId : val } }

	//--FOR STATS--
	int reportComparisonCount = 0; //number of times a report by it was compared by a report by self and result treated as a report on it
	int reportComparisonTrueCount = 0; //number of times a report by it was compared by a report by self and it was a match.
	int msgCount = 0;      		//messages received from veh
	int trueMsgCount = 0; //messages received from veh which were evaluated to true by self
};
typedef std::tr1::unordered_map<int, vehStats*> int2vehStats;

struct reportsGist {
	int reportedTrueCount = 0;
	int reportedCount = 0;
	int_2_bool messages;
};
typedef std::tr1::unordered_map<int, reportsGist*> int2reportsGist;

struct vehStatsEntityCentric {  // { vehId : (true#, total#, { reporterId : (true#, total#, { msgId : val }) }) }
	int reportedTrueCount = 0;
	int reportedCount = 0;
	int2reportsGist reporters; // { reporterId : (true#, total#, { msgId : val }) }
};


typedef std::tr1::unordered_map<int, vehStatsEntityCentric*> int2vehStatsEntityCentric;
typedef std::tr1::unordered_set<int> intSet;
typedef std::tr1::unordered_map<int, intSet*> int_2_intSet;

struct reportsBasket {
	int_2_intSet scope;
	int2vehStatsEntityCentric vehicles;
	inline bool contains(int veh, int msg) {
		return (scope.count(veh) && scope[veh]->count(msg));
	}
	inline bool contains(int veh) {
		return scope.count(veh);
	}
};
