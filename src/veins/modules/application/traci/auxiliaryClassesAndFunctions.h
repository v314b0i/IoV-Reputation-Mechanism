#ifndef MY_AUX_HEADER_FILE
#define MY_AUX_HEADER_FILE

#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <sstream>

//TODO create separate structs for stats collection counters, keep app functionality and ststs collection differentiated.
enum MyApplMessageKinds {
	SEND_INFOMSG_EVT, STAGE_SHIFT_EVT, INFO_MSG, REPORT_MSG
};
typedef std::tr1::unordered_map<int, bool> int_2_bool;
typedef std::tr1::unordered_map<int, float> int_2_float;
typedef std::tr1::unordered_map<int, int> int_2_int;
typedef std::tr1::unordered_set<int> intSet;
typedef std::tr1::unordered_map<int, int_2_float> int_2_int2float;
typedef std::tr1::unordered_map<int, int_2_bool*> int_2_int2bool;
typedef std::tr1::unordered_map<int, intSet*> int_2_intSet;

template<typename keytype, typename valtype, typename objtype> objtype mapValuestoContainer(std::tr1::unordered_map<keytype, valtype> map, objtype &obj);
template<typename keytype, typename valtype, typename objtype> objtype mapKeystoContainer(std::tr1::unordered_map<keytype, valtype> map, objtype &obj);
template<typename keytype, typename valtype> std::tr1::unordered_set<keytype> getKeySet(std::tr1::unordered_map<keytype, valtype> map);
template<typename type> inline float vectorMedian(std::vector<type> vec);
template<typename type> inline float vectorMean(std::vector<type> vec);
template<typename type> inline float vectorMode(std::vector<type> vec);
template<typename container> float inline standardDeviation(container obj, float centre);
template<typename container> float inline medianAbsoluteDeviation(container obj, float centre);
template<typename type> bool ismultimodal(std::vector<type> vec, float median);
template<typename type> bool ismultimodal(std::vector<type> vec);
intSet csvToIntSet(std::string csvstr);

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

struct vehStatsEntityCentric {  // { vehId : (truefloat#, total#, { reporterId : (true#, total#, { msgId : val }) }) }
	int reportedTrueCount = 0;
	int reportedCount = 0;
	int2reportsGist reporters; // { reporterId : (true#, total#, { msgId : val }) }
};

typedef std::tr1::unordered_map<int, vehStatsEntityCentric*> int2vehStatsEntityCentric;

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
inline int uniqueReportersInBasket(reportsBasket basket);
inline int reportsInBasket(reportsBasket basket);
inline int vehiclesInScope(int_2_intSet scope);
inline int messagesInScope(int_2_intSet scope);

struct msgVal {
	int id;
	float truthValue;
	msgVal(int i, float Val) : id(i), truthValue(Val) {
	}
};
struct vehMsgHistory {
	//can use ordered int->float map for messages instead of vector<{int,float}> but insertion and deletion complexity would be log(n)
	//would have to use it if we allow for older messges to be added that were skipped earlier (highly exceptional cases) but that adds
	//more complexity to the functions, not required for simulation.
	int logLimit;
	int splitFactor;
	int splitLevel;
	int totalMessages;
	std::vector<msgVal> messages;
	int_2_float splitTotals;
	intSet splitSizes;
	vehMsgHistory(int factor, int level) { //for f=5,l=4 : sS=<{5,-1},{25,-1},{125,-1},{625,-1}> , lL=625.
		splitFactor = factor;
		splitLevel = level;
		int size = 1;
		for (int i = 1; i <= level; i++) {
			size *= splitFactor;
			splitSizes.insert(size);
			splitTotals[size] = 0;
		}
		logLimit = size;
	}

	void insert(int_2_float messageValsMap) {
		std::vector<int> keys;
		mapKeystoContainer(messageValsMap, keys);
		std::sort(keys.begin(), keys.end());
		for (auto key : keys) {
			msgVal M(key, messageValsMap[key]);
			insert(M);
		}
	}

	void insert(msgVal M) {
		messages.insert(messages.begin(), M);
		for (auto size : splitSizes) {
			splitTotals[size] += M.truthValue;
			if (messages.size() > size)
				splitTotals[size] -= messages.data()[size].truthValue;
		}
		if (messages.size() > logLimit)
			messages.pop_back();
	}
	int_2_float getSplitAvgs() {
		int_2_float splitAvgs;
		for (auto size : splitSizes)
			if (messages.size() >= size)
				splitAvgs[size] = splitTotals[size] / size;
		return splitAvgs;
	}
};
typedef std::tr1::unordered_map<int, vehMsgHistory*> int2VehMsgHistory;

#endif
