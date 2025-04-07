#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#include "ast.h"
#include "bdd.h"

// Each variable is either a BDD symbol or a variable that represents a binary decision diagram
struct Bvar_ptype {
    // Binary variable type
    std::string name{};
};

struct Bdd_ptype {
    // Expression type
    std::string name{};
    uint32_t id{};
};

using Ptype = std::variant<Bvar_ptype, Bdd_ptype>;
enum class Ptype_type : size_t { BVAR = 0,
                                 BDD = 1 };

class Walker {
    // An instance of the tree-walk interpreter
    // Manages the environment of the interpreter and available BDDs in the memory
   private:
    std::unordered_map<uint32_t, std::unique_ptr<Bdd>> bdd_map;
    std::unordered_map<std::string, Ptype> globals;

    void walk_decl_stmt(const decl_stmt& statement);
    
   public:
    Walker() = default;

    void walk(const stmt& statement);
};
