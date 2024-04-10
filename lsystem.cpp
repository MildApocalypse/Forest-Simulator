#include <iostream>
#include <stack>
#include <string>
#include <vector>

#include "cgra_math.hpp"
#include "lsystem.hpp"

using namespace cgra;
using namespace lsys;
using namespace std;

Rule::Rule() {
	
}

Rule::Rule(char c, string t) {
	character = c;
	transform = t;
	context = RuleContext();
}

Rule::Rule(char c, string t, float ch) {
	character = c;
	transform = t;
	chance = ch;
}

Rule::Rule(char c, string t, RuleContext rc) {
	character = c;
	transform = t;
	context = rc;
}

Rule::Rule(char c, string t, RuleContext rc, float ch) {
	character = c;
	transform = t;
	context = rc;
	chance = ch;
}

Rule::~Rule() {}

bool Rule::isValid(string check) {
	return check != invalid;
}

bool Rule::matches(char characterMatch, string left, string right) {
	bool charMatches = (characterMatch == character);

	if(charMatches) {
		bool leftMatches = isLeftMatch(left);
		bool rightMatches = isRightMatch(right);

		if(isValid(context.left) && isValid(context.right)) {
			return leftMatches && rightMatches;
		} else if(isValid(context.left)) {
			return leftMatches;
		} else if(isValid(context.right)) {
			return rightMatches;
		}
	}

	return charMatches;
}

bool Rule::isLeftMatch(string left) {
	string s;
	if(left != "" && isValid(context.left)) {
		s = getCompleteLeftContext(left);
		if(s.size() > context.left.size()) {
			s = left.substr(context.left.size(), s.size());
		}
	}

	return (s == context.left);
}

bool Rule::isRightMatch(string right) {
	string s;
	if(right != "" && isValid(context.right)) {
		string s = getCompleteRightContext(right).substr(0, context.right.size());
	}

	return (s == context.right);
}

string Rule::getCompleteLeftContext(string s) {
	stack<bool> skipStack;
	bool skip = false;
	string finalString = "";
	for(int i = s.size()-1; i >= 0; i--) {
		char c = s.at(i);

		if(c == ']') {
			skip = true;
			skipStack.push(skip);
		} else if(c == '[') {
			if(skipStack.size() > 0) {
				skipStack.pop();
			}
			if(skipStack.size() == 0) {
				skip = false;
			}
		}

		if(!skip) {
			if(!inIgnore(c)) {
				finalString += c;
			}
		}
	}

	reverse(finalString.begin(), finalString.end());

	return finalString;
}

string Rule::getCompleteRightContext(string s) {
	string finalString = "";
	for(char c : s) {
		if(!inIgnore(c)) {
			finalString += c;
		}
	}
	return finalString;
}

bool Rule::inIgnore(char c) {
	return (find(ignore.begin(), ignore.end(), c) != ignore.end());
}

void Rule::print() {
	cout << "character: " << character << endl;
	cout << "transform: " << transform << endl;
	cout << "chance: " << chance << endl;
	cout << "rightcontext: " << context.right << endl;
	cout << "leftcontext: " << context.left << endl;
}

LSystem::LSystem() {}

LSystem::LSystem(string axiom, vector<Rule> r) {
	strings.push_back(axiom);
	rules = r;
	currentString = axiom;
}

LSystem::~LSystem() {}

vector<string> LSystem::generate() {
	for(int i = 0; i < int(currentString.size()); i++) {
		char c = currentString.at(i);
		string left;
		string right;

		if(i-1 > 0)
			left = currentString.substr(0, i);
		if(i+1 < int(currentString.size()))
		 	right = currentString.substr(i+1, currentString.size());

		bool ruleMatch = false;
		vector<Rule> ruleMatches;
		float probabilitySum = 0.0;

		for(Rule r : rules) {
			if(r.matches(c, left, right)) {
				if(r.chance != 0.0) {
					ruleMatches.push_back(r);
					probabilitySum += r.chance;
				} else {
					ruleMatch = true;
					nextString += r.transform;
					break;
				}
			}
		}

		if(ruleMatches.size() == 0 && !ruleMatch) {
			nextString += c;
		} else if(!ruleMatch) {
			float accumulatedChance = 0.0;
			for(Rule match : ruleMatches) {
				float r = math::random(0.0, 1.0);
				float chance = (match.chance / probabilitySum) + accumulatedChance;

				if(r <= chance) {
					nextString += match.transform;
					break;
				}
				accumulatedChance += chance;
			}
		}
	}

	currentString = nextString;
	nextString = "";

	strings.push_back(currentString);

	return strings;
}