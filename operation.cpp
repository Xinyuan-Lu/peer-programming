#include "operation.h"
#include "json.hpp"

using json = nlohmann::json;

operation::operation(op o, size_t p, std::string c, char t, size_t id): opcode(o), pos(p), context(c), text(t), index(id){
	this->cstrlen = this->context.length();
	// this->tstrlen = this->text.length();
}

operation::operation(std::string str) {
    fromString(str);
}

std::string operation::toString() {
    auto j = json{{"opcode", opcode}, {"pos", pos}, {"text", text}, {"index", index}};
    return j.dump();
}

void operation::fromString(std::string str){
    auto j = json::parse(str);
    this->opcode = j["opcode"];
    this->pos = j["pos"];
    this->text = j["text"];
    this->index = j["index"];
}

void modifystr(){
	switch (this->opcode){
		case INSERT:{
			this->context.insert(this->p, this->text);
			cstrlen = cstrlen + 1;
			break;
		}
		case DELETE:{
			this->context.erase(this->p, 1);
			cstrlen = cstrlen - 1;
			break;
		}
		default:
			std::count<<"Error operation"<<endl;
	}
}

size_t retain(operatin op){
	switch (op.opcode){
		case INSERT:{
			this->pos += 1;
			break;
		}
		case DELETE:{
			this->pos -= 1;
			break;
		}
		default:
			std::count<<"Error operation"<<endl;
	}
}

operation* transform(operation op1, operation op2){
	operation* oparrprime = (struct operation *)malloc(2 * sizeof(struct operation));
	// if(op1.opcode == "INSERT" && op2.opcode == "INSERT"){
	if(op1.index < op2.index){
		if(op1.pos < op2.pos){
			oparrprime[0] = operation(op1.opcode, op1.pos, op1.context, op1.text);
			oparrprime[1] = operation(op2.opcode, op2.retain(op1), op2.context, op2.text);
		}
		if(op1.pos > op2.pos){
			oparrprime[0] = operation(op1.opcode, op1.pos, op1.context, op1.text);
			oparrprime[1] = operation(op2.opcode, op2.pos, op2.context, op2.text);
		}
	}
	else{
		if(op1.pos > op2.pos){
			oparrprime[0] = operation(op1.opcode, op1.retain(op2), op1.context, op1.text);
			oparrprime[1] = operation(op2.opcode, op2.pos, op2.context, op2.text);
		}
		if(op1.pos < op2.pos){
			oparrprime[0] = operation(op1.opcode, op1.pos, op1.context, op1.text);
			oparrprime[1] = operation(op2.opcode, op2.pos, op2.context, op2.text);
		}
	}
	if(op1.pos == op2.pos){
		if(op1.index < op2.index){
			if(op2.opcode == "INSERT"){
				oparrprime[0] = operation(op1.opcode, op1.pos, op1.context, op1.text);
				oparrprime[1] = operation(op2.opcode, op2.retain(op1), op2.context, op2.text);
			}
			if(op2.opcode == "DELETE"){
				if(op1.opcode == "INSERT"){
					oparrprime[0] = operation(op1.opcode, op1.pos, op1.context, op1.text);
					oparrprime[1] = operation(op2.opcode, op2.pos, op2.context, op2.text);
				}
				else{
					oparrprime[0] = operation(op1.opcode, op1.pos, op1.context, op1.text);
					oparrprime[1] = operation(op2.opcode, op2.retain(op1), op2.context, op2.text);
				}
			}
		}
		else{
			if(op1.opcode == "INSERT"){
				oparrprime[0] = operation(op1.opcode, op1.retain(op2), op1.context, op1.text);
				oparrprime[1] = operation(op2.opcode, op2.pos, op2.context, op2.text);
			}
			if(op1.opcode == "DELETE"){
				if(op2.opcode == "INSERT"){
					oparrprime[0] = operation(op1.opcode, op1.pos, op1.context, op1.text);
					oparrprime[1] = operation(op2.opcode, op2.pos, op2.context, op2.text);
				}
				else{
					oparrprime[0] = operation(op1.opcode, op1.retain(op2), op1.context, op1.text);
					oparrprime[1] = operation(op2.opcode, op2.pos, op2.context, op2.text);
				}
			}
		}
	}
	return oparrprime;
}








