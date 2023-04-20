import json
import os
from os.path import join as pjoin

root_dir = pjoin(os.path.dirname(__file__), "../")
grammar_dir = pjoin(root_dir, "grammar")
out_dir = pjoin(root_dir, "src/SqlDbLib/src")

bnf_file = pjoin(grammar_dir, "sql.bnf")
with open(bnf_file, "r") as bnf_file:
    if bnf_file is None:
        print("Error: Can't open file '%s'" % bnf_file)
        exit(1)

    lines = bnf_file.readlines()
    bnf_file.close()
    bnf = {
        "main": "",
        "items": {},
    }

    for i, line in enumerate(lines):
        if line.isspace():
            continue

        eq = line.find("::=")
        if eq == -1:
            print("Error: Invalid line %d" % i)
            continue

        name = line[:eq].lstrip().rstrip()
        value = line[eq + 3:].lstrip().rstrip()

        if name in bnf["items"]:
            raise Exception("Error: Duplicate non-terminal '%s'" % name)

        bnf["items"][name] = {
            "raw_value": value,
            "values": [],
        }

        value_expressions = value.split(" ")
        current_value_array = []
        skip_indices = set()

        for j, value in enumerate(value_expressions):
            if value == "|":
                bnf["items"][name]["values"].append(current_value_array)
                current_value_array = []
                continue
            if j in skip_indices:
                continue
            if value == "E":
                continue

            repeat = None
            repeatable = False
            if value[-1] == "*" or value[-1] == "+" or value[-1] == "?":
                repeatable = True
                repeat = value[-1]
                value = value[1:-2]

            item = {}
            err = False

            if (value.startswith('"') and value.endswith('"')) or (value.startswith("'") and value.endswith("'")) and len(value) > 2:
                item = {
                    "type": "TERM",
                    "value": value[1:-1],
                }
            elif value.startswith("@") and value.endswith("@"):
                item = {
                    "type": "TERM_TYPE",
                    "value": value,
                }
            elif value.startswith("<") and value.endswith(">"):
                item = {
                    "type": "N_TERM",
                    "value": value,
                }
            else:
                err = True

            if repeatable:
                item["repeat"] = repeat

            if err:
                print("Error: don't know how to handle '%s', line %d" % (value, i + 1))
            else:
                current_value_array.append(item)

        bnf["items"][name]["values"].append(current_value_array)

    bnf["main"] = lines[0][:lines[0].find("::=")].lstrip().rstrip()

    # Validate all non-terminals are defined
    errors = []
    for name, value in bnf["items"].items():
        if name.startswith("@"):
            continue
        for value_array in value["values"]:
            for value in value_array:
                if value["type"] == "N_TERM":
                    if value["value"] not in bnf["items"]:
                        errors.append("Error: Non-terminal '%s' is not defined" % value["value"])
    if len(errors) > 0:
        for error in errors:
            print(error)
        exit(1)


def toPascalCase(s):
    return s.replace("_", " ").title().replace(" ", "")


with open(pjoin(out_dir, "AutogenParser.h"), "w") as out:
    def o(s):
        out.write(s + "\n")

    o('#pragma once')
    o('')
    o('#include <SqlDbLib/Parser.h>')
    o('#include "ParserPrivate.h"')
    o('')
    o('#include <deque>')
    o('#include <stack>')
    o('')
    o('// #define AUTOGEN_PARSER_DEBUG')
    o('')

    # # Generate struct FWD
    # for i, nterm_name in enumerate(bnf["items"]):
    #     o('struct SqlParseTree_%s;' % toPascalCase(nterm_name[1:-1]))
    #
    # o('')
    #
    # # Generate structs
    # for i, nterm_name in enumerate(bnf["items"]):
    #     o('struct SqlParseTree_%s : public SqlAstNode' % toPascalCase(nterm_name[1:-1]))
    #     o('{')
    #     o('    int value_set = 0;')
    #     o('    union ValueSet')
    #     o('    {')
    #     for value_set in bnf["items"][nterm_name]["values"]:
    #         o('        struct {')
    #         for value in value_set:
    #             if value["type"] == "N_TERM":
    #                 o('            SqlParseTree_%s *v_%s;' % (toPascalCase(value["value"][1:-1]), value["value"][1:-1]))
    #         o('        };')
    #     o('    } value;')
    #     o('};')
    #     o('')

    # Generate declarations
    for i, nterm_name in enumerate(bnf["items"]):
        args = "std::deque<SqlToken *> &q"
        if nterm_name != bnf["main"]:
            args += ", SqlAstNode *parent"
        o('SqlAstNode *SqlParseTree_Parse_%s(%s);' % (toPascalCase(nterm_name[1:-1]), args))

with open(pjoin(out_dir, "AutogenParser.cpp"), "w") as out:
    def o(s):
        out.write(s + "\n")

    o('#include <SqlDbLib/Parser.h>')
    o('')
    o('#include "AutogenParser.h"')
    o('#include "ParserPrivate.h"')
    o('')
    o('#include <memory>')
    o('#include <queue>')

    o('')
    o('#define TRY_PARSE_TERM(x) \\')
    o('    { \\')
    o('        if (q.empty()) break; \\')
    o('        token = q.front(); \\')
    o('        if (strncmp(token->buffer, x, sizeof(x) - 1) != 0) \\')
    o('            break; \\')
    o('        q.pop_front(); \\')
    o('        SqlAstNode *term_node = new SqlAstNode(); \\')
    o('        term_node->type = "TERM"; \\')
    o('        term_node->parent = node.get(); \\')
    o('        term_node->value = std::string(token->buffer, sizeof(x) - 1); \\')
    o('        node->children.push_back(term_node); \\')
    o('    }')
    o('')
    o('#define TRY_PARSE_TERM_0_1(x) \\')
    o('    { \\')
    o('        if (q.empty()) break; \\')
    o('        token = q.front(); \\')
    o('        if (strncmp(token->buffer, x, sizeof(x) - 1) == 0) \\')
    o('        { \\')
    o('            q.pop_front(); \\')
    o('            std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \\')
    o('            term_node->type = "TERM"; \\')
    o('            term_node->value = std::string(token->buffer, sizeof(x) - 1); \\')
    o('            node->children.push_back(term_node.release()); \\')
    o('        } \\')
    o('    }')
    o('')
    o('#define TRY_PARSE_TERM_0_N(x) \\')
    o('    { \\')
    o('        while (true) \\')
    o('        { \\')
    o('            if (q.empty()) break; \\')
    o('            token = q.front(); \\')
    o('            if (strncmp(token->buffer, x, sizeof(x) - 1) != 0) \\')
    o('                break; \\')
    o('            q.pop_front(); \\')
    o('            std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \\')
    o('            term_node->type = "TERM"; \\')
    o('            term_node->value = std::string(token->buffer, sizeof(x) - 1); \\')
    o('            node->children.push_back(term_node.release()); \\')
    o('        } \\')
    o('    }')
    o('')
    o('#define TRY_PARSE_TERM_1_N(x) \\')
    o('    { \\')
    o('        if (q.empty()) break; \\')
    o('        token = q.front(); \\')
    o('        if (strncmp(token->buffer, x, sizeof(x) - 1) != 0) \\')
    o('            break; \\')
    o('        q.pop_front(); \\')
    o('        std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \\')
    o('        term_node->type = "TERM"; \\')
    o('        term_node->value = std::string(token->buffer, sizeof(x) - 1); \\')
    o('        node->children.push_back(term_node.release()); \\')
    o('        while (true) \\')
    o('        { \\')
    o('            token = q.front(); \\')
    o('            if (strncmp(token->buffer, x, sizeof(x) - 1) != 0) \\')
    o('                break; \\')
    o('            q.pop_front(); \\')
    o('            std::unique_ptr<SqlAstNode> term_node_n = std::make_unique<SqlAstNode>(); \\')
    o('            term_node_n->type = "TERM"; \\')
    o('            term_node_n->value = std::string(token->buffer, sizeof(x) - 1); \\')
    o('            node->children.push_back(term_node_n.release()); \\')
    o('        } \\')
    o('    }')

    o('')
    o('#define TRY_PARSE_TERM_TYPE(x) \\')
    o('    { \\')
    o('        if (q.empty()) break; \\')
    o('        token = q.front(); \\')
    o('        if (token->type != SQL_TOKEN_TYPE_##x) \\')
    o('            break; \\')
    o('        q.pop_front(); \\')
    o('        SqlAstNode *term_node = new SqlAstNode(); \\')
    o('        term_node->type = "TERM"; \\')
    o('        term_node->parent = node.get(); \\')
    o('        term_node->value = std::string(token->buffer, token->size); \\')
    o('        node->children.push_back(term_node); \\')
    o('    }')
    o('')
    o('#define TRY_PARSE_TERM_TYPE_0_1(x) \\')
    o('    { \\')
    o('        if (q.empty()) break; \\')
    o('        token = q.front(); \\')
    o('        if (token->type == SQL_TOKEN_TYPE_##x) \\')
    o('        { \\')
    o('            q.pop_front(); \\')
    o('            std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \\')
    o('            term_node->type = "TERM"; \\')
    o('            term_node->value = std::string(token->buffer, token->size); \\')
    o('            node->children.push_back(term_node.release()); \\')
    o('        } \\')
    o('    }')
    o('')
    o('#define TRY_PARSE_TERM_TYPE_0_N(x) \\')
    o('    { \\')
    o('        while (true) \\')
    o('        { \\')
    o('            if (q.empty()) break; \\')
    o('            token = q.front(); \\')
    o('            if (token->type != SQL_TOKEN_TYPE_##x) \\')
    o('                break; \\')
    o('            q.pop_front(); \\')
    o('            std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \\')
    o('            term_node->type = "TERM"; \\')
    o('            term_node->value = std::string(token->buffer, token->size); \\')
    o('            node->children.push_back(term_node.release()); \\')
    o('        } \\')
    o('    }')
    o('')
    o('#define TRY_PARSE_TERM_TYPE_1_N(x) \\')
    o('    { \\')
    o('        if (q.empty()) break; \\')
    o('        token = q.front(); \\')
    o('        if (token->type != SQL_TOKEN_TYPE_##x) \\')
    o('            break; \\')
    o('        q.pop_front(); \\')
    o('        std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \\')
    o('        term_node->type = "TERM"; \\')
    o('        term_node->value = std::string(token->buffer, token->size); \\')
    o('        node->children.push_back(term_node.release()); \\')
    o('        while (true) \\')
    o('        { \\')
    o('            token = q.front(); \\')
    o('            if (token->type != SQL_TOKEN_TYPE_##x) \\')
    o('                break; \\')
    o('            q.pop_front(); \\')
    o('            std::unique_ptr<SqlAstNode> term_node_n = std::make_unique<SqlAstNode>(); \\')
    o('            term_node_n->type = "TERM"; \\')
    o('            term_node_n->value = std::string(token->buffer, sizeof(x) - 1); \\')
    o('            node->children.push_back(term_node_n.release()); \\')
    o('        } \\')
    o('    }')

    o('')
    o('#define TRY_PARSE_NTERM(x) \\')
    o('    { \\')
    o('        SqlAstNode *nterm_node = SqlParseTree_Parse_##x(q, node.get()); \\')
    o('        if (nterm_node == nullptr) \\')
    o('            break; \\')
    o('        node->children.push_back(nterm_node); \\')
    o('    }')
    o('')
    o('#define TRY_PARSE_NTERM_0_1(x) \\')
    o('    { \\')
    o('        SqlAstNode *nterm_node = SqlParseTree_Parse_##x(q, node.get()); \\')
    o('        if (nterm_node != nullptr) \\')
    o('            node->children.push_back(nterm_node); \\')
    o('    }')
    o('')
    o('#define TRY_PARSE_NTERM_0_N(x) \\')
    o('    { \\')
    o('        while (true) \\')
    o('        { \\')
    o('            SqlAstNode *nterm_node = SqlParseTree_Parse_##x(q, node.get()); \\')
    o('            if (nterm_node == nullptr) \\')
    o('                break; \\')
    o('            node->children.push_back(nterm_node); \\')
    o('        } \\')
    o('    }')
    o('')
    o('#define TRY_PARSE_NTERM_1_N(x) \\')
    o('    { \\')
    o('        SqlAstNode *nterm_node = SqlParseTree_Parse_##x(q, node.get()); \\')
    o('        if (nterm_node == nullptr) \\')
    o('            break; \\')
    o('        node->children.push_back(nterm_node); \\')
    o('        while (true) \\')
    o('        { \\')
    o('            SqlAstNode *nterm_node_n = SqlParseTree_Parse_##x(q, node.get()); \\')
    o('            if (nterm_node_n == nullptr) \\')
    o('                break; \\')
    o('            node->children.push_back(nterm_node_n); \\')
    o('        } \\')
    o('    }')

    # Generate definitions
    for i, nterm_name in enumerate(bnf["items"]):
        is_main = True
        args = "std::deque<SqlToken *> &q"
        if nterm_name != bnf["main"]:
            args += ", SqlAstNode *parent"
            is_main = False

        entry = bnf["items"][nterm_name]
        entry_name = toPascalCase(nterm_name[1:-1])

        o('SqlAstNode *SqlParseTree_Parse_%s(%s)' % (entry_name, args))
        o('{')
        o('    SqlToken *token = nullptr;')
        o('')
        o('    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();')
        o('    node->type = "%s";' % entry_name)
        if not is_main:
            o('    node->parent = parent;')
        o('')

        o('    std::deque<SqlToken *> q_copy = q;')

        for value_set in entry["values"]:
            o('    do')
            o('    {')
            o('        #ifdef AUTOGEN_PARSER_DEBUG')
            o('            printf("Entered: Value set %s\\n");' % entry_name)
            o('        #endif')
            o('        q = q_copy;')
            for value in value_set:
                o('        // %s%s (%s)' % (value["value"], value["repeat"] if "repeat" in value else "", value["type"]))

                has_repeat = "repeat" in value
                if has_repeat:
                    zero_or_one = value["repeat"] == "?"
                    zero_or_more = value["repeat"] == "*"
                    one_or_more = value["repeat"] == "+"

                if value["type"] == "TERM":
                    key = value["value"].replace('"', '\\"')
                elif value["type"] == "TERM_TYPE":
                    key = value["value"][1:-1]
                elif value["type"] == "N_TERM":
                    key = toPascalCase(value["value"][1:-1])

                o('')
                o('        #ifdef AUTOGEN_PARSER_DEBUG')
                o('            printf("    Trying : Token %s                     %%s\\n", q.empty() ? "<empty queue>" : q.front()->buffer);' % key)
                o('        #endif')
                if not has_repeat:
                    if value["type"] == "TERM":
                        o('        TRY_PARSE_TERM("%s");' % key)
                    elif value["type"] == "TERM_TYPE":
                        o('        TRY_PARSE_TERM_TYPE(%s);' % key)
                    elif value["type"] == "N_TERM":
                        o('        TRY_PARSE_NTERM(%s);' % key)
                elif zero_or_one:
                    if value["type"] == "TERM":
                        o('        TRY_PARSE_TERM_0_1("%s");' % key)
                    elif value["type"] == "TERM_TYPE":
                        o('        TRY_PARSE_TERM_TYPE_0_1(%s);' % key)
                    elif value["type"] == "N_TERM":
                        o('        TRY_PARSE_NTERM_0_1(%s);' % key)
                elif zero_or_more:
                    if value["type"] == "TERM":
                        o('        TRY_PARSE_TERM_0_N("%s");' % key)
                    elif value["type"] == "TERM_TYPE":
                        o('        TRY_PARSE_TERM_TYPE_0_N(%s);' % key)
                    elif value["type"] == "N_TERM":
                        o('        TRY_PARSE_NTERM_0_N(%s);' % key)
                elif one_or_more:
                    if value["type"] == "TERM":
                        o('        TRY_PARSE_TERM_1_N("%s");' % key)
                    elif value["type"] == "TERM_TYPE":
                        o('        TRY_PARSE_TERM_TYPE_1_N(%s);' % key)
                    elif value["type"] == "N_TERM":
                        o('        TRY_PARSE_NTERM_1_N(%s);' % key)
                o('        #ifdef AUTOGEN_PARSER_DEBUG')
                o('            printf("    Success: %s succeeded\\n");' % key)
                o('        #endif')
                o('')
            o('        #ifdef AUTOGEN_PARSER_DEBUG')
            o('            printf("Success: Value set %s\\n");' % entry_name)
            o('        #endif')
            o('        goto success;')
            o('    } while (0);')
            o('    #ifdef AUTOGEN_PARSER_DEBUG')
            o('        printf("Fail   : Value set %s failed\\n");' % entry_name)
            o('    #endif')
            o('    for (SqlAstNode *child : node->children)')
            o('        delete child;')
            o('    node->children.clear();')
            o('')
        o('    fail:')
        o('    q = q_copy;')
        o('    return nullptr;')
        o('')
        o('    success:')
        o('    return node.release();')
        o('}')
        o('')
