#include <tr1/unordered_map>

typedef std::tr1::unordered_map<int, bool> reporterId_2_val;
typedef std::tr1::unordered_map<int, reporterId_2_val*> msgId_2_reporterId2val;
class vehStats {
public:
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
typedef std::tr1::unordered_map<int, vehStats*> int2vehStats;

