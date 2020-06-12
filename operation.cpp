#include "operation.h"
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

static bool check(operation op);

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
	j = json{
		{"ops", p.ops}, 
		{"versionNumber", p.versionNumber}, 
		{"senderID", p.senderID}, 
		{"baseLength", p.baseLength},
		{"targetLength", p.targetLength},
	};
}

void from_json(const json& j, operation& p) {
	j.at("ops").get_to(p.ops);
	j.at("versionNumber").get_to(p.versionNumber);
	j.at("senderID").get_to(p.senderID);
	j.at("baseLength").get_to(p.baseLength);
	j.at("targetLength").get_to(p.targetLength);
}

operation::operation(int versionNumber, int senderID){
	this->versionNumber = versionNumber;
	this->senderID = senderID;
	baseLength = 0;
	targetLength = 0;
}

operation::operation(std::string str) {
    auto j = json::parse(str);
	j.get_to(*this);
}

std::string operation::toString() {
    json j = *this;
    return j.dump() + "\n";
}

operation::operation(int senderID, size_t baseLength, size_t pos, std::string& insertString){
	this->baseLength = this->targetLength = 0;
	if(pos > 0){
        subop retainOp1(RETAIN, pos, "");
        this->retain(retainOp1);
    }
	subop ins(INSERT, pos, insertString);
    this->insert(ins);
    if(baseLength-pos != 0){
        subop retainOp2(RETAIN, baseLength-pos, "");
        this->retain(retainOp2);
    }
	this->senderID = senderID;
}

operation::operation(int senderID, size_t baseLength, size_t pos, size_t deleteLength){
	this->baseLength = this->targetLength = 0;
	if(pos > 0){
        subop retainOp1(RETAIN, pos, "");
        this->retain(retainOp1);
    }
	subop del(DELETE, deleteLength, "");
    this->remove(del);
	if(baseLength - pos - deleteLength > 0){
		subop retainOp2(RETAIN, baseLength-pos-deleteLength, "");
		this->retain(retainOp2);
	}
	if(baseLength - pos - deleteLength  < 0){
		throw std::runtime_error("You are deleting more than you have");
	}
	this->senderID = senderID;        
}
//operation::compose(subop op)

// Assume op1, op2, $$context$$
std::vector<operation> operation::transform(operation& op1, operation& op2){
	
	check(op1);
	check(op2);
	
	// Both operation take place in the same edit space.
	assert(op1.baseLength == op2.baseLength);

	std::vector<operation> operprime(2, operation(0, 0));
	operprime[0].senderID = op1.senderID;
	operprime[1].senderID = op2.senderID;

	size_t i1 = 0, i2 = 0;
	std::vector<subop> ops1 = op1.ops;
	std::vector<subop> ops2 = op2.ops;
	while(true){
		if ((i1 >= ops1.size()) && (i2 >= ops2.size())) {
			break;
		}
		if (i1 < ops1.size() && ops1[i1].opcode == INSERT) {
			operprime[0].insert(ops1[i1]);
			subop tempop(RETAIN, ops1[i1].text.length(), "");
			operprime[1].retain(tempop);
			i1++;
			continue;
		}
		if (i2 < ops2.size() && ops2[i2].opcode == INSERT) {
			subop tempop = subop(RETAIN, ops2[i2].text.length(), "");
			operprime[0].retain(tempop);
			operprime[1].insert(ops2[i2]);
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
			}else if (ops1[i1].offset == ops2[i2].offset){
				minl.offset = ops2[i2].offset;
				i1++;
				i2++;
			}else{
				minl.offset = ops1[i1].offset;
				ops2[i2].offset = ops2[i2].offset - ops1[i1].offset;
				i1++;
			}
			
			operprime[0].remove(minl);
		}else if (ops1[i1].opcode == RETAIN && ops2[i2].opcode == DELETE){
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
	check(operprime[0]);
	check(operprime[1]);
	return operprime;
}

std::string operation::applyTransform(std::string context){
	//std::cout << context.size() << " " << baseLength << std::endl;
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
		ops.rbegin()->offset += operand.offset;
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
		if (ops.size() > 1 && (ops.rbegin() + 1)->opcode == INSERT) {
			(ops.rbegin() + 1)->text += operand.text;
		} else {
			ops.push_back(*ops.rbegin());
			//ops.rbegin() = (ops.rbegin() + 1);
			*(ops.rbegin() + 1) = operand;
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
	
	check(op);
	//Sender ID and my ID should be the same if called correctly
	assert(op.senderID == this->senderID);
	// The base length of the second operation has to be the target length of the first operation
	assert(this->targetLength == op.baseLength);

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
	check(*newOp);
	return newOp;
}

static bool check(operation op) {
	size_t base = 0;
	size_t target = 0;
	for(auto sub: op.ops){
		switch (sub.opcode)
		{
		case INSERT:
			//assert(sub.offset == sub.text.size());
			target +=  sub.text.length();
			break;
		case DELETE:
			base += sub.offset;
			break;
		case RETAIN:
			base += sub.offset;
			target += sub.offset;
			break;
		default:
			throw std::runtime_error("No such opcode");
		}

	}
	if(base != op.baseLength){
		std::cout << op.toString()<< std::endl;
		throw std::runtime_error("Base length wrong");
		return false;
	}
	if(target != op.targetLength){
		std::cout << op.toString()<< std::endl;
		throw std::runtime_error("target length wrong");
		return false;
	}
	return true;
}

// std::vector<operation> operation::transform(operation& op1, operation& op2){

// }