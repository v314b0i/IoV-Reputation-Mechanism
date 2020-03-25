//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
cplusplus {{
#include "veins/base/utils/Coord.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/base/utils/SimpleAddress.h"
#include "string"
}}

namespace veins;

class BaseFrame1609_4;
class noncobject Coord;
class LAddress::L2Type extends void;

packet reportDumpMsg extends BaseFrame1609_4 {
    LAddress::L2Type senderAddress = -1;
	LAddress::L2Type reporteeAddress = -1;
	LAddress::L2Type primaryRecipientAddress = -1; //for stats collection only. DO NOT USE recipientAddress (from parent class) or messages will be dropped by others.
	string trueMsgs; //commaseperated list of messages from reportee found to be true by sender
	string falseMsgs;//commaseperated list of messages from reportee found to be false by sender
}
