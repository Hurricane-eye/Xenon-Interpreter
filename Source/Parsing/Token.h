#pragma once

#include "Stable.h"

//   T: Non-keyword tokens
//   K: Keyword tokens
#define TOKEN_LIST(T, K)												\
	/* End of source indicator. */										\
	T(EOS, "EOS", 0)													\
																		\
	/* Punctuators. */													\
	T(LPAREN, "(", 0)													\
	T(RPAREN, ")", 0)													\
	T(LBRACE, "{", 0)													\
	T(RBRACE, "}", 0)													\
	T(SEMICOLON, ";", 0)												\
	T(PERIOD, ".", 0)													\
																		\
	/* Assignment operators. */											\
	/* IsAssignmentOp() relies on this block of enum values being */	\
	/* contiguous and sorted in the same order! */						\
	T(INIT, "=init", 2) /* AST-use only. */								\
	T(ASSIGN, "=", 2)													\
																		\
	/* Binary operators sorted by precedence. */						\
	/* IsBinaryOp() relies on this block of enum values */				\
	/* being contiguous and sorted in the same order! */				\
	T(COMMA, ",", 1)													\
	T(OR, "||", 4)														\
	T(AND, "&&", 5)														\
	T(ADD, "+", 12)														\
	T(SUB, "-", 12)														\
	T(MUL, "*", 13)														\
	T(DIV, "/", 13)														\
	T(MOD, "%", 13)														\
	/* String Operators */												\
	T(STRING_CONCAT, "$", 13)										    \
	T(STRING_DELETE , "#", 13)											\
																		\
	/* Compare operators sorted by precedence. */						\
	/* IsCompareOp() relies on this block of enum values */				\
	/* being contiguous and sorted in the same order! */				\
	T(EQ, "==", 9)														\
	T(NE, "<>", 9)														\
	T(LT, "<", 10)														\
	T(GT, ">", 10)														\
	T(LTE, "<=", 10)													\
	T(GTE, ">=", 10)													\
																		\
	/* Unary operators. */												\
	/* IsUnaryOp() relies on this block of enum values */				\
	/* being contiguous and sorted in the same order! */				\
	T(NOT, "!", 0)														\
	K(VOID, "void", 0)													\
																		\
	K(ELSE, "else", 0)													\
	K(IF, "if", 0)														\
	K(RETURN, "return", 0)												\
	K(INT, "int", 0)													\
	K(REAL, "real", 0)													\
	K(STRING, "string", 0)												\
	K(OUT, "out", 0)													\
	K(IN, "in", 0)														\
	K(WHILE, "while", 0)												\
																		\
	/* Literals. */														\
	T(INTEGER_LITERAL, NULL, 0)											\
	T(REAL_LITERAL, NULL, 0)											\
	T(STRING_LITERAL, NULL, 0)											\
																		\
	/* Identifiers (not keywords). */									\
	T(IDENTIFIER, NULL, 0)												\
																		\
	/* Illegal token - not able to scan. */								\
	T(ILLEGAL, "ILLEGAL", 0)											\
	T(ESCAPED_KEYWORD, NULL, 0)											\
																		\
	/* Scanner-internal use only. */									\
	T(WHITESPACE, NULL, 0)												\
	T(UNINITIALIZED, NULL, 0)											

class Token {
public:
#define T(name, string, precedence) name,
	enum Type {
		TOKEN_LIST(T, T)
		NUM_TOKENS
	};
#undef T

public:
	Token(Type t, const std::string &v = "");

	Token &operator=(const Token &rhs) {
		type = rhs.type;
		value = rhs.value;
	}

	Type type;
	std::string value;

public:
	// Returns a string corresponding to the C++ token name
	// (e.g. "LT" for the token LT).
	static const char* Name(Type tok) {
		return names_[tok];
	}

	// Predicates
	static bool IsKeyword(Type tok) {
		return token_types_[tok] == 'K';
	}

	static bool IsIdentifier(Type tok) {
		return tok == IDENTIFIER;
	}

	static bool IsAssignmentOp(Type tok) {
		return INIT <= tok && tok <= ASSIGN;
	}

	static bool IsBinaryOp(Type op) { return COMMA <= op && op <= STRING_DELETE; }

	static bool IsCompareOp(Type op) {
		return EQ <= op && op <= GTE;
	}

	static bool IsOrderedRelationalCompareOp(Type op) {
		return op == LT || op == LTE || op == GT || op == GTE;
	}

	static bool IsEqualityOp(Type op) {
		return op == EQ;
	}

	static bool IsInequalityOp(Type op) {
		return op == NE;
	}

	static bool IsArithmeticCompareOp(Type op) {
		return IsOrderedRelationalCompareOp(op) ||
			IsEqualityOp(op) || IsInequalityOp(op);
	}

	static Type NegateCompareOp(Type op) {
		switch (op) {
		case EQ: return NE;
		case NE: return EQ;
		case LT: return GTE;
		case GT: return LTE;
		case LTE: return GT;
		case GTE: return LT;
		}
	}

	static Type ReverseCompareOp(Type op) {
		switch (op) {
		case EQ: return EQ;
		case NE: return NE;
		case LT: return GT;
		case GT: return LT;
		case LTE: return GTE;
		case GTE: return LTE;
		}
	}

	static bool EvalComparison(Type op, double op1, double op2) {
		switch (op) {
		case Token::EQ: return (op1 == op2);
		case Token::NE: return (op1 != op2);
		case Token::LT: return (op1 < op2);
		case Token::GT: return (op1 > op2);
		case Token::LTE: return (op1 <= op2);
		case Token::GTE: return (op1 >= op2);
		}
	}

	static bool IsUnaryOp(Type op) {
		return (NOT <= op && op <= VOID) || op == ADD || op == SUB;
	}

	// Returns a string corresponding to the min-C token string
	// (.e., "<" for the token LT) or NULL if the token doesn't
	// have a (unique) string (e.g. an IDENTIFIER).
	static const char* String(Type tok) {
		return strings_[tok];
	}

	// Returns a enum value corresponding to the min-C token string
	//(.e., Token::INT for "int"
	static Token::Type GetValue(const std::string &string);

	static bool Includes(const std::string &string);
	static bool Includes(Token::Type value);

	static uint8_t StringLength(Type tok) {
		return string_lengths_[tok];
	}

	// Returns the precedence > 0 for binary and compare
	// operators; returns 0 otherwise.
	static int Precedence(Type tok) {
		return precedences_[tok];
	}

private:
	static const char* const names_[NUM_TOKENS];
	static const char* const strings_[NUM_TOKENS];
	static const uint8_t string_lengths_[NUM_TOKENS];
	static const int8_t precedences_[NUM_TOKENS];
	static const char token_types_[NUM_TOKENS];
};
