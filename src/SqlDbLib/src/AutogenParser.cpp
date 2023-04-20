#include <SqlDbLib/Parser.h>

#include "AutogenParser.h"
#include "ParserPrivate.h"

#include <memory>
#include <queue>

#define TRY_PARSE_TERM(x) \
    { \
        if (q.empty()) break; \
        token = q.front(); \
        if (strncmp(token->buffer, x, sizeof(x) - 1) != 0) \
            break; \
        q.pop_front(); \
        SqlAstNode *term_node = new SqlAstNode(); \
        term_node->type = "TERM"; \
        term_node->parent = node.get(); \
        term_node->value = std::string(token->buffer, sizeof(x) - 1); \
        node->children.push_back(term_node); \
    }

#define TRY_PARSE_TERM_0_1(x) \
    { \
        if (q.empty()) break; \
        token = q.front(); \
        if (strncmp(token->buffer, x, sizeof(x) - 1) == 0) \
        { \
            q.pop_front(); \
            std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \
            term_node->type = "TERM"; \
            term_node->value = std::string(token->buffer, sizeof(x) - 1); \
            node->children.push_back(term_node.release()); \
        } \
    }

#define TRY_PARSE_TERM_0_N(x) \
    { \
        while (true) \
        { \
            if (q.empty()) break; \
            token = q.front(); \
            if (strncmp(token->buffer, x, sizeof(x) - 1) != 0) \
                break; \
            q.pop_front(); \
            std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \
            term_node->type = "TERM"; \
            term_node->value = std::string(token->buffer, sizeof(x) - 1); \
            node->children.push_back(term_node.release()); \
        } \
    }

#define TRY_PARSE_TERM_1_N(x) \
    { \
        if (q.empty()) break; \
        token = q.front(); \
        if (strncmp(token->buffer, x, sizeof(x) - 1) != 0) \
            break; \
        q.pop_front(); \
        std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \
        term_node->type = "TERM"; \
        term_node->value = std::string(token->buffer, sizeof(x) - 1); \
        node->children.push_back(term_node.release()); \
        while (true) \
        { \
            token = q.front(); \
            if (strncmp(token->buffer, x, sizeof(x) - 1) != 0) \
                break; \
            q.pop_front(); \
            std::unique_ptr<SqlAstNode> term_node_n = std::make_unique<SqlAstNode>(); \
            term_node_n->type = "TERM"; \
            term_node_n->value = std::string(token->buffer, sizeof(x) - 1); \
            node->children.push_back(term_node_n.release()); \
        } \
    }

#define TRY_PARSE_TERM_TYPE(x) \
    { \
        if (q.empty()) break; \
        token = q.front(); \
        if (token->type != SQL_TOKEN_TYPE_##x) \
            break; \
        q.pop_front(); \
        SqlAstNode *term_node = new SqlAstNode(); \
        term_node->type = "TERM"; \
        term_node->parent = node.get(); \
        term_node->value = std::string(token->buffer, token->size); \
        node->children.push_back(term_node); \
    }

#define TRY_PARSE_TERM_TYPE_0_1(x) \
    { \
        if (q.empty()) break; \
        token = q.front(); \
        if (token->type == SQL_TOKEN_TYPE_##x) \
        { \
            q.pop_front(); \
            std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \
            term_node->type = "TERM"; \
            term_node->value = std::string(token->buffer, token->size); \
            node->children.push_back(term_node.release()); \
        } \
    }

#define TRY_PARSE_TERM_TYPE_0_N(x) \
    { \
        while (true) \
        { \
            if (q.empty()) break; \
            token = q.front(); \
            if (token->type != SQL_TOKEN_TYPE_##x) \
                break; \
            q.pop_front(); \
            std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \
            term_node->type = "TERM"; \
            term_node->value = std::string(token->buffer, token->size); \
            node->children.push_back(term_node.release()); \
        } \
    }

#define TRY_PARSE_TERM_TYPE_1_N(x) \
    { \
        if (q.empty()) break; \
        token = q.front(); \
        if (token->type != SQL_TOKEN_TYPE_##x) \
            break; \
        q.pop_front(); \
        std::unique_ptr<SqlAstNode> term_node = std::make_unique<SqlAstNode>(); \
        term_node->type = "TERM"; \
        term_node->value = std::string(token->buffer, token->size); \
        node->children.push_back(term_node.release()); \
        while (true) \
        { \
            token = q.front(); \
            if (token->type != SQL_TOKEN_TYPE_##x) \
                break; \
            q.pop_front(); \
            std::unique_ptr<SqlAstNode> term_node_n = std::make_unique<SqlAstNode>(); \
            term_node_n->type = "TERM"; \
            term_node_n->value = std::string(token->buffer, sizeof(x) - 1); \
            node->children.push_back(term_node_n.release()); \
        } \
    }

#define TRY_PARSE_NTERM(x) \
    { \
        SqlAstNode *nterm_node = SqlParseTree_Parse_##x(q, node.get()); \
        if (nterm_node == nullptr) \
            break; \
        node->children.push_back(nterm_node); \
    }

#define TRY_PARSE_NTERM_0_1(x) \
    { \
        SqlAstNode *nterm_node = SqlParseTree_Parse_##x(q, node.get()); \
        if (nterm_node != nullptr) \
            node->children.push_back(nterm_node); \
    }

#define TRY_PARSE_NTERM_0_N(x) \
    { \
        while (true) \
        { \
            SqlAstNode *nterm_node = SqlParseTree_Parse_##x(q, node.get()); \
            if (nterm_node == nullptr) \
                break; \
            node->children.push_back(nterm_node); \
        } \
    }

#define TRY_PARSE_NTERM_1_N(x) \
    { \
        SqlAstNode *nterm_node = SqlParseTree_Parse_##x(q, node.get()); \
        if (nterm_node == nullptr) \
            break; \
        node->children.push_back(nterm_node); \
        while (true) \
        { \
            SqlAstNode *nterm_node_n = SqlParseTree_Parse_##x(q, node.get()); \
            if (nterm_node_n == nullptr) \
                break; \
            node->children.push_back(nterm_node_n); \
        } \
    }
SqlAstNode *SqlParseTree_Parse_Root(std::deque<SqlToken *> &q)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Root";

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Root\n");
        #endif
        q = q_copy;
        // <queries> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Queries                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Queries);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Queries succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Root\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Root failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Query(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Query";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Query\n");
        #endif
        q = q_copy;
        // <select_query> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token SelectQuery                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(SelectQuery);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: SelectQuery succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Query\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Query failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Query\n");
        #endif
        q = q_copy;
        // <insert_query> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token InsertQuery                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(InsertQuery);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: InsertQuery succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Query\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Query failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Query\n");
        #endif
        q = q_copy;
        // <update_query> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token UpdateQuery                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(UpdateQuery);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: UpdateQuery succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Query\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Query failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Query\n");
        #endif
        q = q_copy;
        // <delete_query> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token DeleteQuery                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(DeleteQuery);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: DeleteQuery succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Query\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Query failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Queries(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Queries";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Queries\n");
        #endif
        q = q_copy;
        // <query> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Query                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Query);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Query succeeded\n");
        #endif

        // ; (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token ;                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM(";");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: ; succeeded\n");
        #endif

        // <query_list>* (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token QueryList                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM_0_N(QueryList);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: QueryList succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Queries\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Queries failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Queries\n");
        #endif
        q = q_copy;
        // <query> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Query                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Query);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Query succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Queries\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Queries failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_QueryList(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "QueryList";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set QueryList\n");
        #endif
        q = q_copy;
        // <queries> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Queries                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Queries);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Queries succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set QueryList\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set QueryList failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_SelectQuery(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "SelectQuery";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set SelectQuery\n");
        #endif
        q = q_copy;
        // <select> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Select                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Select);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Select succeeded\n");
        #endif

        // <from> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token From                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(From);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: From succeeded\n");
        #endif

        // <where>? (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Where                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM_0_1(Where);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Where succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set SelectQuery\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set SelectQuery failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Select(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Select";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Select\n");
        #endif
        q = q_copy;
        // SELECT (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token SELECT                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("SELECT");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: SELECT succeeded\n");
        #endif

        // <columns>? (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Columns                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM_0_1(Columns);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Columns succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Select\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Select failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_From(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "From";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set From\n");
        #endif
        q = q_copy;
        // FROM (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token FROM                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("FROM");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: FROM succeeded\n");
        #endif

        // <tables> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Tables                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Tables);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Tables succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set From\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set From failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Where(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Where";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Where\n");
        #endif
        q = q_copy;
        // WHERE (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token WHERE                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("WHERE");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: WHERE succeeded\n");
        #endif

        // <cmp_compound> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token CmpCompound                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(CmpCompound);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: CmpCompound succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Where\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Where failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_InsertQuery(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "InsertQuery";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set InsertQuery\n");
        #endif
        q = q_copy;
        // INSERT (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token INSERT                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("INSERT");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: INSERT succeeded\n");
        #endif

        // INTO (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token INTO                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("INTO");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: INTO succeeded\n");
        #endif

        // <table> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Table                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Table);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Table succeeded\n");
        #endif

        // ( (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token (                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("(");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: ( succeeded\n");
        #endif

        // <columns> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Columns                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Columns);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Columns succeeded\n");
        #endif

        // ) (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token )                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM(")");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: ) succeeded\n");
        #endif

        // VALUES (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token VALUES                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("VALUES");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: VALUES succeeded\n");
        #endif

        // ( (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token (                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("(");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: ( succeeded\n");
        #endif

        // <values> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Values                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Values);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Values succeeded\n");
        #endif

        // ) (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token )                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM(")");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: ) succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set InsertQuery\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set InsertQuery failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_UpdateQuery(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "UpdateQuery";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set UpdateQuery\n");
        #endif
        q = q_copy;
        // UPDATE (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token UPDATE                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("UPDATE");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: UPDATE succeeded\n");
        #endif

        // <table> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Table                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Table);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Table succeeded\n");
        #endif

        // SET (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token SET                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("SET");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: SET succeeded\n");
        #endif

        // <set> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Set                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Set);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Set succeeded\n");
        #endif

        // <where>? (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Where                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM_0_1(Where);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Where succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set UpdateQuery\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set UpdateQuery failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_DeleteQuery(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "DeleteQuery";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set DeleteQuery\n");
        #endif
        q = q_copy;
        // DELETE (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token DELETE                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("DELETE");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: DELETE succeeded\n");
        #endif

        // FROM (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token FROM                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("FROM");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: FROM succeeded\n");
        #endif

        // <table> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Table                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Table);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Table succeeded\n");
        #endif

        // <where>? (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Where                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM_0_1(Where);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Where succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set DeleteQuery\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set DeleteQuery failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Set(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Set";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Set\n");
        #endif
        q = q_copy;
        // <column> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Column                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Column);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Column succeeded\n");
        #endif

        // = (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token =                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("=");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: = succeeded\n");
        #endif

        // <value> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Value                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Value);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Value succeeded\n");
        #endif

        // <comma_set>* (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token CommaSet                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM_0_N(CommaSet);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: CommaSet succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Set\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Set failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_CommaSet(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "CommaSet";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set CommaSet\n");
        #endif
        q = q_copy;
        // , (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token ,                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM(",");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: , succeeded\n");
        #endif

        // <set> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Set                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Set);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Set succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set CommaSet\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set CommaSet failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Column(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Column";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Column\n");
        #endif
        q = q_copy;
        // <identifier> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Identifier                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Identifier);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Identifier succeeded\n");
        #endif

        // <period_column>* (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token PeriodColumn                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM_0_N(PeriodColumn);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: PeriodColumn succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Column\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Column failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Column\n");
        #endif
        q = q_copy;
        // ` (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token `                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("`");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: ` succeeded\n");
        #endif

        // <identifier> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Identifier                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Identifier);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Identifier succeeded\n");
        #endif

        // ` (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token `                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("`");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: ` succeeded\n");
        #endif

        // <period_column>* (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token PeriodColumn                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM_0_N(PeriodColumn);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: PeriodColumn succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Column\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Column failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Columns(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Columns";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Columns\n");
        #endif
        q = q_copy;
        // <column> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Column                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Column);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Column succeeded\n");
        #endif

        // <comma_columns>* (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token CommaColumns                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM_0_N(CommaColumns);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: CommaColumns succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Columns\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Columns failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_CommaColumns(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "CommaColumns";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set CommaColumns\n");
        #endif
        q = q_copy;
        // , (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token ,                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM(",");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: , succeeded\n");
        #endif

        // <columns> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Columns                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Columns);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Columns succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set CommaColumns\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set CommaColumns failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_PeriodColumn(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "PeriodColumn";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set PeriodColumn\n");
        #endif
        q = q_copy;
        // . (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token .                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM(".");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: . succeeded\n");
        #endif

        // <identifier> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Identifier                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Identifier);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Identifier succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set PeriodColumn\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set PeriodColumn failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Table(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Table";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Table\n");
        #endif
        q = q_copy;
        // <identifier> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Identifier                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Identifier);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Identifier succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Table\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Table failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Table\n");
        #endif
        q = q_copy;
        // ` (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token `                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("`");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: ` succeeded\n");
        #endif

        // <identifier> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Identifier                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Identifier);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Identifier succeeded\n");
        #endif

        // ` (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token `                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("`");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: ` succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Table\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Table failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Tables(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Tables";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Tables\n");
        #endif
        q = q_copy;
        // <table> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Table                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Table);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Table succeeded\n");
        #endif

        // <comma_tables>* (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token CommaTables                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM_0_N(CommaTables);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: CommaTables succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Tables\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Tables failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_CommaTables(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "CommaTables";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set CommaTables\n");
        #endif
        q = q_copy;
        // , (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token ,                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM(",");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: , succeeded\n");
        #endif

        // <tables> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Tables                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Tables);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Tables succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set CommaTables\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set CommaTables failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_CmpCompound(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "CmpCompound";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set CmpCompound\n");
        #endif
        q = q_copy;
        // <cmp> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Cmp                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Cmp);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Cmp succeeded\n");
        #endif

        // AND (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token AND                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("AND");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: AND succeeded\n");
        #endif

        // <cmp_compound> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token CmpCompound                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(CmpCompound);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: CmpCompound succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set CmpCompound\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set CmpCompound failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set CmpCompound\n");
        #endif
        q = q_copy;
        // <cmp> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Cmp                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Cmp);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Cmp succeeded\n");
        #endif

        // OR (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token OR                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM("OR");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: OR succeeded\n");
        #endif

        // <cmp_compound> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token CmpCompound                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(CmpCompound);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: CmpCompound succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set CmpCompound\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set CmpCompound failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set CmpCompound\n");
        #endif
        q = q_copy;
        // <cmp> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Cmp                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Cmp);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Cmp succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set CmpCompound\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set CmpCompound failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Cmp(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Cmp";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Cmp\n");
        #endif
        q = q_copy;
        // <column> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Column                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Column);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Column succeeded\n");
        #endif

        // <operator> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Operator                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Operator);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Operator succeeded\n");
        #endif

        // <value> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Value                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Value);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Value succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Cmp\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Cmp failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Value(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Value";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Value\n");
        #endif
        q = q_copy;
        // <string> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token String                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(String);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: String succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Value\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Value failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Value\n");
        #endif
        q = q_copy;
        // <number> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Number                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Number);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Number succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Value\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Value failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Value\n");
        #endif
        q = q_copy;
        // <identifier> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Identifier                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Identifier);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Identifier succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Value\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Value failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Value\n");
        #endif
        q = q_copy;
        // <boolean> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Boolean                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Boolean);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Boolean succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Value\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Value failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Values(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Values";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Values\n");
        #endif
        q = q_copy;
        // <value> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Value                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Value);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Value succeeded\n");
        #endif

        // <value_list_comma>* (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token ValueListComma                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM_0_N(ValueListComma);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: ValueListComma succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Values\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Values failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_ValueListComma(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "ValueListComma";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set ValueListComma\n");
        #endif
        q = q_copy;
        // , (TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token ,                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM(",");
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: , succeeded\n");
        #endif

        // <values> (N_TERM)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token Values                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_NTERM(Values);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: Values succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set ValueListComma\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set ValueListComma failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_String(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "String";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set String\n");
        #endif
        q = q_copy;
        // @STRING@ (TERM_TYPE)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token STRING                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM_TYPE(STRING);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: STRING succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set String\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set String failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Identifier(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Identifier";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Identifier\n");
        #endif
        q = q_copy;
        // @IDENTIFIER@ (TERM_TYPE)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token IDENTIFIER                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM_TYPE(IDENTIFIER);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: IDENTIFIER succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Identifier\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Identifier failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Number(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Number";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Number\n");
        #endif
        q = q_copy;
        // @NUMBER@ (TERM_TYPE)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token NUMBER                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM_TYPE(NUMBER);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: NUMBER succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Number\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Number failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Boolean(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Boolean";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Boolean\n");
        #endif
        q = q_copy;
        // @BOOLEAN@ (TERM_TYPE)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token BOOLEAN                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM_TYPE(BOOLEAN);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: BOOLEAN succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Boolean\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Boolean failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Keyword(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Keyword";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Keyword\n");
        #endif
        q = q_copy;
        // @KEYWORD@ (TERM_TYPE)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token KEYWORD                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM_TYPE(KEYWORD);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: KEYWORD succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Keyword\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Keyword failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

SqlAstNode *SqlParseTree_Parse_Operator(std::deque<SqlToken *> &q, SqlAstNode *parent)
{
    SqlToken *token = nullptr;

    std::unique_ptr<SqlAstNode> node = std::make_unique<SqlAstNode>();
    node->type = "Operator";
    node->parent = parent;

    std::deque<SqlToken *> q_copy = q;
    do
    {
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Entered: Value set Operator\n");
        #endif
        q = q_copy;
        // @OPERATOR@ (TERM_TYPE)

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Trying : Token OPERATOR                     %s\n", q.empty() ? "<empty queue>" : q.front()->buffer);
        #endif
        TRY_PARSE_TERM_TYPE(OPERATOR);
        #ifdef AUTOGEN_PARSER_DEBUG
            printf("    Success: OPERATOR succeeded\n");
        #endif

        #ifdef AUTOGEN_PARSER_DEBUG
            printf("Success: Value set Operator\n");
        #endif
        goto success;
    } while (0);
    #ifdef AUTOGEN_PARSER_DEBUG
        printf("Fail   : Value set Operator failed\n");
    #endif
    for (SqlAstNode *child : node->children)
        delete child;
    node->children.clear();

    fail:
    q = q_copy;
    return nullptr;

    success:
    return node.release();
}

