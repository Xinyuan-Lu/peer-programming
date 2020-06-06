#include "operation.h"
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

void to_json(json& j, const subop& p) {
    j = json{{"opcode", p.opcode}, {"offset", p.offset}, {"text", p.text}};
}

void from_json(const json& j, subop& p) {
    j.at("opcode").get_to(p.opcode);
    j.at("offset").get_to(p.offset);
	j.at("text").get_to(p.text);
}

subop::subop(){}
subop::subop(op opcode, size_t offset, std::string text):opcode(opcode), offset(offset), text(text){}

void to_json(json& j, const operation& p) {
	j = json{{"ops", p.ops}, {"versionNumber", p.versionNumber}, {"senderID", p.senderID}};
}

void from_json(const json& j, operation& p) {
	j.at("ops").get_to(p.ops);
	j.at("versionNumber").get_to(p.versionNumber);
	j.at("senderID").get_to(p.senderID);
}

operation::operation(int versionNumber, int senderID){
	this->versionNumber = versionNumber;
	this->senderID = senderID;
	
	baseLength = 0;
}

operation::operation(std::string str) {
    auto j = json::parse(str);
	j.get_to(*this);
	baseLength = 0;
}

std::string operation::toString() {
    json j = *this;
    return j.dump() + "\n";
}




//operation::compose(subop op)

// Assume op1, op2, $$context$$
std::vector<operation> operation::transform(operation& op1, operation& op2){
	
	// Both operation take place in the same edit space.
	assert(op1.baseLength == op2.baseLength);

	std::vector<operation> operprime;
	size_t i1 = 0, i2 = 0;
	std::vector<subop> ops1 = op1.ops;
	std::vector<subop> ops2 = op2.ops;
	while(true){
		if ((i1 >= ops1.size()) && (i2 >= ops2.size())) {
			break;
		}
		if (i1 < ops1.size() && ops1[i1].opcode == INSERT) {
			operprime[0].insert(ops1[i1]);
			operprime[1].retain(ops1[i1]);
			i1++;
			continue;
		}
		if (i2 < ops2.size() && ops2[i2].opcode == INSERT) {
			operprime[0].insert(ops2[i2]);
			operprime[1].retain(ops2[i2]);
			i2++;
			continue;
		}
		if ((i1 >= ops1.size()) || (i2 >= ops2.size())) {
			assert(false);
		}
		subop minl;
		if (ops1[i1].opcode == RETAIN && ops2[i2].opcode == RETAIN) {
			if (ops1[i1].offset > ops2[i2].offset){
				minl = ops2[i2];
				ops1[i1].offset = ops1[i1].offset - ops2[i2].offset;
				i2++;
			}else if (ops1[i1].offset == ops2[i2].offset){
				minl = ops1[i1];
				i1++;
				i2++;
			}else{
				minl = ops1[i1];
				ops2[i2].offset = ops2[i2].offset - ops1[i1].offset;
				i1++;
			}
			operprime[0].retain(minl);
			operprime[1].retain(minl);
		}else if (ops1[i1].opcode == DELETE && ops2[i2].opcode == DELETE){
			if (ops1[i1].offset > ops2[i2].offset){
				ops1[i1].offset -= ops2[i2].offset;
				i2++;
			}else if (ops1[i1].offset == ops2[i2].offset){
				i1++;
				i2++;
			}else{
				ops2[i2].offset -= ops1[i1].offset;
				i1++;
			}
		}else if (ops1[i1].opcode == DELETE && ops2[i2].opcode == RETAIN){
			minl.opcode = DELETE;
			if (ops1[i1].offset > ops2[i2].offset) {
				minl.offset = ops2[i2].offset;
				minl.text = "";
				ops1[i1].offset = ops1[i1].offset - ops2[i2].offset;
				i2++;
			}else if (ops1[i1].offset > ops2[i2].offset){
				minl = ops2[i2];
				i1++;
				i2++;
			}else{
				minl = ops1[i1];
				ops2[i2].offset = ops2[i2].offset - ops1[i1].offset;
				i1++;
			}
			operprime[0].remove(minl);
		}else if (ops1[i1].offset == RETAIN && ops2[i2].opcode == DELETE){
			minl.opcode = DELETE;
			if(ops1[i1].offset > ops2[i2].offset){
				minl.offset = ops2[i2].offset;
				ops1[i1].offset -= ops2[i2].offset;
				i2++;
			}else if(ops1[i1].offset == ops2[i2].offset){
				minl.offset = ops1[i1].offset;
				i1++;
				i2++;
			}else{
				minl.offset = ops1[i1].offset;
				ops2[i2].offset = ops2[i2].offset - ops1[i1].offset;
				i1++;
			}
			operprime[1].remove(minl);
		}else{
			//Theses are not compatable operations
			assert(false);
		}
	}
	return operprime;
}

std::string operation::applyTransform(std::string context){
	std::cout << context.size() << " " << baseLength << std::endl;
	assert(context.size() == baseLength);	
	size_t strIndex = 0;
	std::string newContext;
	for (auto op : ops) {
		if (op.opcode == RETAIN) {
			assert(strIndex + op.offset <= context.size());
			newContext += context.substr(strIndex, op.offset);
			strIndex += op.offset;
		} else if (op.opcode == INSERT) {
			newContext += op.text;
		} else {
			strIndex += op.offset;
		}
	}
	assert(strIndex == context.size());
	return newContext;
}


void operation::remove(subop& operand){
	assert(operand.opcode == DELETE);
	if (operand.offset == 0) {
		return;
	}
	baseLength += operand.offset;
	if (!ops.empty() && ops.rbegin()->opcode == DELETE) {
		ops.rbegin()->offset -= operand.offset;
	} else {
		ops.push_back(operand);
	}
}

void operation::insert(subop &operand) {
	assert(operand.opcode == INSERT);
	if (operand.text == "")	{
		return;
	}
	targetLength += operand.text.length();
	if (!ops.empty() && ops.rbegin()->opcode == INSERT)	{
		ops.rbegin()->text += operand.text;
	} else if (!ops.empty() && ops.rbegin()->opcode == DELETE) {
		if ((ops.rbegin() + 1)->opcode == INSERT) {
			(ops.rbegin() + 1)->text += operand.text;
		} else {
			ops.rbegin() = (ops.rbegin() + 1);
			(ops.rbegin() + 2)->text = operand.text;
		}
	} else {
		ops.push_back(operand);
	}
}

void operation::retain(subop& operand){
	assert(operand.opcode == RETAIN);
	if (operand.offset == 0){
		return;
	}
	this->baseLength += operand.offset;
	this->targetLength += operand.offset;
	if(!this->ops.empty() && this->ops.rbegin()->opcode == RETAIN){
		this->ops.rbegin()->offset += operand.offset;
	}else{
		this->ops.push_back(operand);
	}
}

// this-> is the procedding operation and op is the following op
// return value will be a smart pointer of operation.
std::shared_ptr<operation> operation::compose(operation& op){
	//Sender ID and my ID should be the same if called correctly
	assert(op.senderID == this->senderID);
	// The base length of the second operation has to be the target length of the first operation
	assert(this->targetLength == op.targetLength);

	std::shared_ptr<operation> newOp(new operation(this->versionNumber, this->senderID));

	std::vector<subop> ops1 = this->ops;
	std::vector<subop> ops2 = op.ops;

	size_t i1 = 0, i2 = 0;

	while(true){
		if ((i1 >= ops1.size()) && (i2 >= ops2.size())) {
			break;
		}
		if (i1 < ops1.size() && ops1[i1].opcode == DELETE) {
			newOp->remove(ops[i1++]);
			continue;
		}
		if (i2 < ops2.size() && ops2[i2].opcode == INSERT) {
			newOp->insert(ops2[i2++]);
			continue;
		}
		if ((i1 >= ops1.size()) || (i2 >= ops2.size())) {
			assert(false);
		}
		if (ops1[i1].opcode == RETAIN && ops2[i2].opcode == RETAIN) {
			if (ops1[i1].offset > ops2[i2].offset) {
				newOp->retain(ops2[i2]);
				ops1[i1].offset -= ops2[i2].offset;
				i2++;
			}else if (ops1[i1].offset == ops2[i2].offset){
				newOp->retain(ops1[i1]);
				i1++;
				i2++;
			}else{
				newOp->retain(ops1[i1]);
				ops2[i2].offset -= ops1[i1].offset;
				i1++;
			}	
		}else if (ops1[i1].opcode == INSERT && ops2[i2].opcode == DELETE){
			if (ops1[i1].text.length() > ops2[i2].offset){
				size_t cutlength = ops1[i1].text.length() - ops2[i2].offset;
				ops1[i1].text = ops1[i1].text.substr(ops2[i2].offset, cutlength);
				i2++;
			}else if (ops1[i1].text.length() == ops2[i2].offset){
				i1++;
				i2++;
			}else{
				ops2[i2].offset = ops2[i2].offset - ops1[i1].text.length();
				i1++;
			}
		}else if (ops1[i1].opcode == INSERT && ops2[i2].opcode == RETAIN){
			if (ops1[i1].text.length() > ops2[i2].offset){
				std::string tempstr = ops1[i1].text.substr(0, ops2[i2].offset);
				subop tempop = subop(ops[i1].opcode, ops2[i2].offset, tempstr);
				newOp->insert(tempop);
				size_t cutlength = ops1[i1].text.length() - ops2[i2].offset;
				ops1[i1].text = ops1[i1].text.substr(ops2[i2].offset, cutlength);
				i2++;
			}else if (ops1[i1].text.length() == ops2[i2].offset){
				newOp -> insert(ops1[i1]);
				i1++;
				i2++;
			} else {
				newOp -> insert(ops1[i1]);
				ops2[i2].offset = ops2[i2].offset - ops1[i1].text.length();
				i1++;
			}
		}else if(ops1[i1].opcode == RETAIN && ops2[i2].opcode == DELETE){
			if (ops1[i1].offset > ops2[i2].offset){
				newOp->remove(ops2[i2]);
				ops1[i1].offset -= ops2[i2].offset;
				i2++;
			}else if(ops1[i1].offset == ops2[i2].offset){
				newOp->remove(ops2[i2]);
				i2++;
				i1++;
			}else{
				subop tempop = subop(DELETE, ops1[i1].offset, "");
				newOp->remove(tempop);
				ops2[i2].offset -= ops1[i1].offset;
				i1++;
			}
		}else{
			// This should not be possible.
			assert(false);
		}		
	}
	return newOp;
}

// std::vector<operation> operation::transform(operation& op1, operation& op2){

// }