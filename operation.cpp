#include "operation.h"
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

operation::operation(op o, size_t p, std::string t, size_t id): opcode(o), pos(p), text(t), index(id){}

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

// Assume op1, op2, $$context$$
operation* transformation::transform(operation op1, operation op2){
	operation* oparrprime = (struct operation *)malloc(2 * sizeof(struct operation));
	if(op1.index < op2.index){
		if(op1.pos < op2.pos){
			oparrprime[0] = operation(op1);
			oparrprime[1] = operation(op2.opcode, retain(op1), op2.text, op2.index);
		}
		if(op1.pos > op2.pos){
			oparrprime[0] = operation(op1);
			oparrprime[1] = operation(op2);
		}
	}
	else{
		if(op1.pos > op2.pos){
			oparrprime[0] = operation(op1.opcode, retain(op2), op1.text, op1.index);
			oparrprime[1] = operation(op2);
		}
		if(op1.pos < op2.pos){
			oparrprime[0] = operation(op1);
			oparrprime[1] = operation(op2);
		}
	}
	if(op1.pos == op2.pos){
		if(op1.index < op2.index){
			if(op2.opcode == INSERT){
				oparrprime[0] = operation(op1);
				oparrprime[1] = operation(op2.opcode, retain(op1), op2.text, op2.index);
			}
			if(op2.opcode == DELETE){
				if(op1.opcode == INSERT){
					oparrprime[0] = operation(op1);
					oparrprime[1] = operation(op2);
				}
				else{
					oparrprime[0] = operation(op1);
					oparrprime[1] = operation(op2.opcode, retain(op1), op2.text, op2.index);
				}
			}
		}
		else{
			if(op1.opcode == INSERT){
				oparrprime[0] = operation(op1.opcode, retain(op2), op1.text, op1.index);
				oparrprime[1] = operation(op2);
			}
			if(op1.opcode == DELETE){
				if(op2.opcode == INSERT){
					oparrprime[0] = operation(op1);
					oparrprime[1] = operation(op2);
				}
				else{
					oparrprime[0] = operation(op1.opcode, retain(op2), op1.text, op1.index);
					oparrprime[1] = operation(op2);
				}
			}
		}
	}
	return oparrprime;
}

std::string transformation::applyTransform(operation op, std::string context){
	switch (op.opcode){
		case INSERT:
			context.insert(op.pos, op.text);
			break;
		case DELETE:
			context.erase(op.pos, 1);
			break;
		default:
			std::cout<<"Error operation"<<std::endl;
	}
	return context;
}

size_t transformation::retain(operation op){
	switch (op.opcode){
		case INSERT:{
			op.pos += 1;
			break;
		}
		case DELETE:{
			op.pos -= 1;
			break;
		}
		default:
			std::cout<<"Error operation"<< std::endl;
	}
	return op.pos;
}

// void operation::modifystr(){
// 	switch (opcode){
// 		case INSERT:{
// 			context.insert(pos, text);
// 			cstrlen = cstrlen + 1;
// 			break;
// 		}
// 		case DELETE:{
// 			context.erase(pos, 1);
// 			cstrlen =cstrlen - 1;
// 			break;
// 		}
// 		default:
// 			std::cout<<"Error operation"<<std::endl;
// 	}
// }
// size_t operation::retain(operation op){
// 	switch (op.opcode){
// 		case INSERT:{
// 			size_t temp;
// 			temp = pos;
// 			temp = temp + 1;
// 			pos = temp;
// 			break;
// 		}
// 		case DELETE:{
// 			size_t temp;
// 			temp = pos;
// 			temp = temp - 1;
// 			pos = temp;
// 			break;
// 		}
// 		default:
// 			std::cout<<"Error operation"<< std::endl;
// 	}
// 	return pos;
// }

// operation* operation::transform(operation op1, operation op2){
// 	
// }