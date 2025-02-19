//
// Created by Om Patel on 11/27/2024.
//

#include <iostream>
#include <map>
#include <climits>
#include <cstdint>
#include "compiler.h"
#include "lexer.h"

using namespace std;

// Global lexer and token
LexicalAnalyzer lexer;
Token t;

// Map for storing variable locations
map<string, int> inputMap;

struct InstructionNode* body;

// Forward declarations
void parse_grammer();
void parse_var_sec();
void parse_idList();
int parse_value();
void parse_num_list();

InstructionNode * parse_body();
InstructionNode * parse_stmt_list();
InstructionNode * parse_stmt();
InstructionNode * parse_assign_stmt();
InstructionNode * parse_expresion();
InstructionNode* parse_input_stmt();
InstructionNode* parse_output_stmt();
InstructionNode * parse_while_stmt();
InstructionNode * parse_if_stmt();
InstructionNode * condition_checker();
InstructionNode * parse_for_stmt();
InstructionNode * parse_switch_stmt(InstructionNode* empty);
InstructionNode * parse_all_cases(int location, InstructionNode* empty);
InstructionNode * parse_case(int location);
InstructionNode * parse_edge_case();


struct InstructionNode * parse_generate_intermediate_representation()
{
    parse_grammer();
    return body;        // return starting nde of IR
}

Token get_next_andUnget() {         // Helper func
    Token temp = lexer.GetToken();
    lexer.UngetToken(1);

    return temp;
}

Token expect(TokenType expected) {
    t = lexer.GetToken();
    if (t.token_type != expected) {

        exit(1);
    }
    return t;
}


void parse_grammer() { // program -> var_section body inputs
    t = get_next_andUnget();
    if (t.token_type != ID){
        exit(1);
    }
    parse_var_sec();

    t = get_next_andUnget();
    if (t.token_type != LBRACE){
        exit(1);
    }
    body = parse_body();

    t = get_next_andUnget();
    if (t.token_type != NUM){
        exit(1);
    }
    parse_num_list();
}

void parse_var_sec() // handles var section -> idList SEMICOLON
{
    parse_idList();
    expect(SEMICOLON);
}

void parse_idList()
{
    t = expect(ID);
    inputMap[t.lexeme] = next_available;
    mem[next_available++] = 0;

    t = lexer.GetToken();       // Checking for more Ids if there
    if (t.token_type == COMMA) {
        parse_idList();
    } else {
        lexer.UngetToken(1);
    }
}

struct InstructionNode* parse_body()    // handles -> '{' stmt_list '}'
{
    expect(LBRACE);
    struct InstructionNode* list = parse_stmt_list();
    expect(RBRACE);
    return list;
}

struct InstructionNode* parse_stmt_list(){
    struct InstructionNode* first;
    struct InstructionNode* second;

    first = parse_stmt();   // pasre single statement

    Token temp = get_next_andUnget();   // Peek next token lol
    switch (temp.token_type) {
        case ID:
        case NUM:
        case WHILE:
        case IF:
        case FOR:
        case SWITCH:
        case OUTPUT:
        case INPUT: {
            second = parse_stmt_list();
            auto endList = first;
            while (endList->next != NULL) {
                endList = endList->next;
            }
            endList->next = second;
        }
        default:
            break;
    }

    return first;
}

struct InstructionNode* parse_stmt() // to parse statement
{
    struct InstructionNode* inst = NULL;
    struct InstructionNode* endList;
    auto* empty = new InstructionNode;  // Used for switch cases NOOP

    Token temp = get_next_andUnget();
    switch (temp.token_type) {
        case ID:
            inst = parse_assign_stmt();
            break;
        case WHILE:
            inst = parse_while_stmt();
            break;
        case IF:
            inst = parse_if_stmt();
            break;
        case SWITCH:
            empty->type = NOOP;
            empty->next = NULL;
            inst = parse_switch_stmt(empty);
            endList = inst;
            while (endList->next != NULL)
                endList = endList->next;
            endList->next = empty;
            break;
        case FOR:
            inst = parse_for_stmt();
            break;
        case OUTPUT:
            inst = parse_output_stmt();
            break;
        case INPUT:
            inst = parse_input_stmt();
            break;
        default:
            exit(1);
    }

    return inst;
}

// assign_stmt -> ID EQUAL primary SEMICOLON | ID EQUAL expr SEMICOLON
struct InstructionNode* parse_assign_stmt() {
    auto* assign = new InstructionNode;
    assign->type = ASSIGN;

    t = expect(ID);
    assign->assign_inst.left_hand_side_index = inputMap[t.lexeme];
    expect(EQUAL);

    Token t1 = lexer.GetToken();
    Token t2 = lexer.GetToken();
    lexer.UngetToken(1);
    lexer.UngetToken(1);

    if (t1.token_type == ID || t1.token_type == NUM) {
        if (t2.token_type == PLUS || t2.token_type == MINUS || t2.token_type == MULT || t2.token_type == DIV) {
            auto temp = parse_expresion();
            assign->assign_inst.operand1_index = temp->assign_inst.operand1_index;
            assign->assign_inst.op = temp->assign_inst.op;
            assign->assign_inst.operand2_index = temp->assign_inst.operand2_index;
        } else if (t2.token_type == SEMICOLON) {
            assign->assign_inst.op = OPERATOR_NONE;
            assign->assign_inst.operand1_index = parse_value();
        } else {
            exit(1);
        }
    } else {
        exit(1);
    }

    expect(SEMICOLON);
    assign->next = NULL;
    return assign;
}

// expr -> primary op primary
struct InstructionNode* parse_expresion() {
    auto *getInfo = new InstructionNode;
    getInfo->assign_inst.operand1_index = parse_value();

    Token tok = lexer.GetToken();
    if (tok.token_type != PLUS && tok.token_type != MINUS && tok.token_type != MULT && tok.token_type != DIV) {
        exit(1);
    }

    switch (tok.token_type) {
        case PLUS:
            getInfo->assign_inst.op = OPERATOR_PLUS;
            break;
        case MINUS:
            getInfo->assign_inst.op = OPERATOR_MINUS;
            break;
        case MULT:
            getInfo->assign_inst.op = OPERATOR_MULT;
            break;
        case DIV:
            getInfo->assign_inst.op = OPERATOR_DIV;
            break;
        default:
            // should not reach here
            exit(1);
            break;
    }

    getInfo->assign_inst.operand2_index = parse_value();
    return getInfo;
}


int parse_value(){ // primary → ID | NUM
    Token temp = lexer.GetToken();

    if (temp.token_type != ID && temp.token_type != NUM) {
        exit(1);
    }

    int index;

    if (temp.token_type == ID) {
        index = inputMap[temp.lexeme];
    } else
    {
        index = next_available;
        mem[next_available++] = stoi(temp.lexeme);
    }

    return index;
}

InstructionNode* parse_input_stmt() {
    // let's assume that 'INPUT'  is already confirm before calling parse_input_stmt()
    expect(INPUT);
    InstructionNode* inst = new InstructionNode;

    inst->type = IN;

    t = expect(ID);
    inst->input_inst.var_index = inputMap[t.lexeme];

    expect(SEMICOLON);
    inst->next = NULL;
    return inst;
}

InstructionNode* parse_output_stmt() {
    // lets assume 'OUTPUT' is already confirm before calling pare_output_stmt()
    expect(OUTPUT);
    InstructionNode* inst = new InstructionNode;
    inst->type = OUT;

    t = expect(ID);
    inst->output_inst.var_index = inputMap[t.lexeme];

    expect(SEMICOLON);
    inst->next = NULL;

    return inst;
}


struct InstructionNode* parse_while_stmt() {
    auto *whileStmt = new InstructionNode;
    expect(WHILE);

    whileStmt->type = CJMP;

    struct InstructionNode* condition = condition_checker();

    whileStmt->cjmp_inst.operand1_index = condition->cjmp_inst.operand1_index;
    whileStmt->cjmp_inst.condition_op = condition->cjmp_inst.condition_op;
    whileStmt->cjmp_inst.operand2_index = condition->cjmp_inst.operand2_index;

    Token temp = get_next_andUnget();
    if (temp.token_type != LBRACE)
    {
        exit(1);
    }

    whileStmt->next = parse_body();

    auto* jmp = new InstructionNode;
    jmp->type = JMP;
    jmp->jmp_inst.target = whileStmt;

    auto* noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = NULL;

    struct InstructionNode* getLast = whileStmt;
    while (getLast->next != NULL)
    {
        getLast = getLast->next;
    }

    getLast->next = jmp;
    jmp->next = noop;

    whileStmt->cjmp_inst.target = noop;

    return whileStmt;
}

struct InstructionNode* parse_if_stmt() {
    auto *ifStmt = new InstructionNode;
    expect(IF);

    ifStmt->type = CJMP;
    struct InstructionNode* temp = condition_checker();

    ifStmt->cjmp_inst.operand1_index = temp->cjmp_inst.operand1_index;
    ifStmt->cjmp_inst.condition_op = temp->cjmp_inst.condition_op;
    ifStmt->cjmp_inst.operand2_index = temp->cjmp_inst.operand2_index;

    Token tok = get_next_andUnget();
    if (tok.token_type != LBRACE) {
        exit(1);
    }

    ifStmt->next = parse_body();

    auto* noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = NULL;

    struct InstructionNode* endList = ifStmt;
    while (endList->next != NULL) {
        endList = endList->next;
    }

    endList->next = noop;
    ifStmt->cjmp_inst.target = noop;

    return ifStmt;
}

// condition → primary relop primary
struct InstructionNode* condition_checker() {
    auto* condition = new InstructionNode;

    Token tok = get_next_andUnget();
    if (tok.token_type != ID && tok.token_type != NUM) {
        cout << "Idhar error hai kya?\n";
        exit(1);
    }

    condition->cjmp_inst.operand1_index = parse_value();

    // parse_relop:
    Token rel = lexer.GetToken();
    if (rel.token_type != GREATER && rel.token_type != LESS && rel.token_type != NOTEQUAL) {
        cout << "error hai?\n";
        exit(1);
    }

    switch (rel.token_type) {
        case GREATER:
            condition->cjmp_inst.condition_op = CONDITION_GREATER;
            break;
        case LESS:
            condition->cjmp_inst.condition_op = CONDITION_LESS;
            break;
        case NOTEQUAL:
            condition->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
            break;
        default:
            cout << "Should not reach here! check code again\n";
            exit(1);
    }

    tok = get_next_andUnget();
    if (tok.token_type != ID && tok.token_type != NUM) {
        cout << "checking if ID aur NUm toh nahi hai\n";
        exit(1);
    }

    condition->cjmp_inst.operand2_index = parse_value();
    return condition;
}

// for_stmt → FOR LPAREN assign_stmt condition SEMICOLON assign_stmt RPAREN body
struct InstructionNode* parse_for_stmt() {
    struct InstructionNode* for_stmt;
    struct InstructionNode* assign;
    expect(FOR);
    expect(LPAREN);

    Token tok = get_next_andUnget();
    if (tok.token_type != ID) {
        exit(1);
    }

    for_stmt = parse_assign_stmt();
    auto* temp = new InstructionNode;
    temp->type = CJMP;

    // Condition of for loop
    struct InstructionNode* temp2 = condition_checker();
    temp->cjmp_inst.operand1_index = temp2->cjmp_inst.operand1_index;
    temp->cjmp_inst.condition_op = temp2->cjmp_inst.condition_op;
    temp->cjmp_inst.operand2_index = temp2->cjmp_inst.operand2_index;

    expect(SEMICOLON);

    tok = get_next_andUnget();
    if (tok.token_type != ID) {
        exit(1);
    }

    assign = parse_assign_stmt();
    assign->next = NULL;
    expect(RPAREN);

    tok = get_next_andUnget();
    if (tok.token_type != LBRACE) {
        cout << "error hai?\n";
        exit(1);
    }

    temp->next = parse_body();

    auto* add_stmt = temp->next;
    while (add_stmt->next != NULL) {
        add_stmt = add_stmt->next;
    }
    add_stmt->next = assign;

    auto* jmp = new InstructionNode;
    jmp->type = JMP;
    jmp->jmp_inst.target = temp;

    auto* empty = new InstructionNode;
    empty->type = NOOP;
    empty->next = NULL;

    jmp->next = empty;

    struct InstructionNode* endList = temp; // Link everything, lol
    while (endList->next != NULL) {
        endList = endList->next;
    }

    endList->next = jmp;
    temp->cjmp_inst.target = empty;
    for_stmt->next = temp;

    return for_stmt;
}

struct InstructionNode * parse_switch_stmt(InstructionNode* empty) // parse_switch_stmt parses SWITCH statements
{
    struct InstructionNode* switchStmt;
    expect(SWITCH);

    t = expect(ID);
    int location = inputMap[t.lexeme];
    expect(LBRACE);

    Token tok = get_next_andUnget();
    if (tok.token_type != CASE) {
        exit(1);
    }

    switchStmt = parse_all_cases(location, empty);

    tok = get_next_andUnget();
    if (tok.token_type == DEFAULT) {
        auto endList = switchStmt;
        while (endList->next->next != NULL) {
            endList = endList->next;
        }
        endList->next = parse_edge_case();
        expect(RBRACE);
    } else if (tok.token_type == RBRACE) {
        lexer.GetToken();
        return switchStmt;
    } else {
        exit(1);
    }

    return switchStmt;
}

struct InstructionNode* parse_all_cases(int location, InstructionNode* empty) {
    struct InstructionNode* caseNode;
    struct InstructionNode* caseList = NULL;

    Token tok = get_next_andUnget();
    if (tok.token_type != CASE) {
        exit(1);
    }

    caseNode = parse_case(location);

    auto* jmp = new InstructionNode;    // JMP instruction to jump to NOOP
    jmp->type = JMP;
    jmp->jmp_inst.target = empty;

    struct InstructionNode* endList = caseNode->cjmp_inst.target;
    while (endList->next->next != NULL) {
        endList = endList->next;
    }
    endList->next = jmp;

    tok = get_next_andUnget();
    if (tok.token_type == CASE) {
        caseList = parse_all_cases(location, empty);
        auto endL = caseNode;
        while (endL->next->next != NULL)
        {
            endL = endL->next;
        }
        endL->next = caseList;
    }

    return caseNode;
}

struct InstructionNode* parse_case(int location) {  // parse_case parses single CASE
    auto* cast_stmt = new InstructionNode;
    expect(CASE);

    cast_stmt->type = CJMP;
    cast_stmt->cjmp_inst.operand1_index = location;

    cast_stmt->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    t = expect(NUM);

    int index = next_available;
    mem[next_available++] = stoi(t.lexeme);
    cast_stmt->cjmp_inst.operand2_index = index;

    expect(COLON);
    Token tok = get_next_andUnget();
    if (tok.token_type != LBRACE) {
        cout << "debugging here\n";
        exit(1);
    }

    cast_stmt->cjmp_inst.target = parse_body();
    auto* noop = new InstructionNode;
    noop->type = NOOP;

    noop->next = NULL;

    auto endList = cast_stmt->cjmp_inst.target;
    while (endList->next != NULL) {
        endList = endList->next;
    }

    cast_stmt->next = noop;

    endList->next = cast_stmt->next;
    return cast_stmt;
}

struct InstructionNode * parse_edge_case()  // parse_edge_case parses default of a switch
{
    expect(DEFAULT);
    expect(COLON);

    Token tok = get_next_andUnget();
    if (tok.token_type != LBRACE) {
        exit(1);
    }

    struct InstructionNode*  defaultInst = parse_body();

    return defaultInst;
}

void parse_num_list() {
    t = expect(NUM);

    inputs.push_back(stoi(t.lexeme));

    Token tok = get_next_andUnget();
    if (tok.token_type == NUM){
        parse_num_list();   // If another number follows, parse again
    } else
    {
        return;
    }
}
