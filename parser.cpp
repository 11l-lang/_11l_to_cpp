#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h> 

#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <functional>
#include <iostream>


class Token
{
public:
	enum Category
	{
		IDENTIFIER = 0,
		KEYWORD = 1,
		DELIMITER = 2,
		OPERATOR = 3,
		NUMERIC_LITERAL = 4,
		STRING_LITERAL = 5,
		SCOPE_BEGIN = 6,
		SCOPE_END = 7,
		STATEMENT_SEPARATOR = 8
	} category;
	const char *value;

	Token(int category, const char *value) : category((Category)category), value(value)
	{
	}

	bool literal() { return category == STRING_LITERAL || category == NUMERIC_LITERAL; }
};

Token tokens[] = { Token(4, "1"), Token(3, "+"), Token(4, "2"), Token(3, "*"), Token(4, "3"), Token(3, "+"), Token(4, "4"), Token(3, "^"), Token(4, "2"), Token(3, "^"), Token(4, "2"), Token(2, "(end)") };
Token *tokenp = &tokens[0];


class SymbolNode;

class SymbolBase
{
public:
	Token::Category category;
	std::string value;
	int lbp;
	std::function<std::shared_ptr<SymbolNode>(std::shared_ptr<SymbolNode>)> nud;
	std::function<std::shared_ptr<SymbolNode>(std::shared_ptr<SymbolNode>, std::shared_ptr<SymbolNode>)> led;
};

class SymbolNode
{
public:
	SymbolBase *symbol;
	std::vector<std::shared_ptr<SymbolNode>> children;
	std::string value;

	std::string to_str()
	{
		if (symbol->category == Token::STRING_LITERAL || symbol->category == Token::NUMERIC_LITERAL)
			return value;
		switch (children.size())
		{
		case 1:
			return "(" + symbol->value + children[0]->to_str() + ")";
		case 2:
			return "(" + children[0]->to_str() + symbol->value + children[1]->to_str() + ")";
		}
		return "";
	}
};

std::map<std::string, SymbolBase> symbol_table;

SymbolBase &symbol(const char *value, int bp = 0)
{
	auto it = symbol_table.find(value);
	if (it == symbol_table.end())
	{
		SymbolBase &sb = symbol_table[value];
		if (strcmp(value, "(literal)") == 0) sb.category = Token::NUMERIC_LITERAL; else sb.category = Token::OPERATOR;
		sb.value = value;
		sb.lbp = bp;
		return sb;
	}
	else
	{
		it->second.lbp = std::max(it->second.lbp, bp);
		return it->second;
	}
}

std::shared_ptr<SymbolNode> token;

class SyntaxError
{
	std::string msg;
public:
	SyntaxError(const std::string &msg) : msg(msg) {}
};

void next_token()
{
	tokenp++;
	if (tokenp == &tokens[_countof(tokens)])
		throw SyntaxError("No more tokens");
	token.reset(new SymbolNode);
	token->symbol = &symbol_table[tokenp->literal() ? "(literal)" : tokenp->value];
	token->value = tokenp->value;
}

void advance(const char *value)
{
	if (value && strcmp(tokenp->value, value) != 0)
		throw SyntaxError("Expected " + std::string(value));
	next_token();
}

std::shared_ptr<SymbolNode> expression(int rbp = 0)
{
	std::shared_ptr<SymbolNode> t = token;
	next_token();
	std::shared_ptr<SymbolNode> left = t->symbol->nud(t);
	while (rbp < token->symbol->lbp)
	{
		t = token;
		next_token();
		left = t->symbol->led(t, left);
	}
	return left;
}

void infix(const char *value, int bp)
{
	symbol(value, bp).led = [bp](std::shared_ptr<SymbolNode> self, std::shared_ptr<SymbolNode> left)->std::shared_ptr<SymbolNode>{
		self->children.clear();
		self->children.push_back(left);
		self->children.push_back(expression(bp));
		return self;
	};
}

void infix_r(const char *value, int bp)
{
	symbol(value, bp).led = [bp](std::shared_ptr<SymbolNode> self, std::shared_ptr<SymbolNode> left)->std::shared_ptr<SymbolNode>{
		self->children.clear();
		self->children.push_back(left);
		self->children.push_back(expression(bp-1));
		return self;
	};
}

void prefix(const char *value, int bp)
{
	symbol(value).nud = [bp](std::shared_ptr<SymbolNode> self)->std::shared_ptr<SymbolNode>{
		self->children.clear();
		self->children.push_back(expression(bp));
		return self;
	};
}

int main(int argc, char* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	infix("+", 110); infix("-", 110);

	infix("*", 120); infix("/", 120);
	infix("%", 120);

	prefix("-", 130); prefix("+", 130); prefix("~", 130);

	infix_r("^", 140);

	symbol("(literal)").nud = [](std::shared_ptr<SymbolNode> self)->std::shared_ptr<SymbolNode>{return self;};

	symbol("(end)");

	symbol(")");

	tokenp--;
	next_token();
	std::cout << expression()->to_str();

	return 0;
}
