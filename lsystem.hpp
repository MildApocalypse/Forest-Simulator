#pragma once

#include <string>
#include <vector>

namespace lsys {

	const std::vector<char> ignore = {
		'[', ']', '+', '-', '\\', '/', '&', '^'
	};

	struct RuleContext {
		std::string left = "Z";
		std::string right = "Z";
	};

	class Rule {
	private:
		char character;
		RuleContext context;
		std::string invalid = "Z";

		bool isValid(std::string);
		bool isLeftMatch(std::string);
		bool isRightMatch(std::string);
		std::string getCompleteLeftContext(std::string);
		std::string getCompleteRightContext(std::string);
		bool inIgnore(char);
	public:
		std::string transform;
		float chance = 0.0;

		Rule();
		Rule(char, std::string);
		Rule(char, std::string, float);
		Rule(char, std::string, RuleContext);
		Rule(char, std::string, RuleContext, float);
		~Rule();
		bool matches(char, std::string, std::string);
		void print();
	};


	class LSystem {
	private:
		std::vector<std::string> strings;
		std::vector<Rule> rules;
		std::string currentString = "";
		std::string nextString = "";
	public:
		LSystem();
		LSystem(std::string, std::vector<Rule>);
		~LSystem();
		std::vector<std::string> generate();
	};
}