#include <queue>

#include "walker.h"

void Walker::clear_memos() {  // for later: Implement garbage collection
    binop_memo.clear();
    not_memo.clear();
    is_sat_memo.clear();
    id_to_expr_memo.clear();
}

void Walker::sweep() {
    std::unordered_set<id_type> preserved_ids{0, 1};
    // Always preserve the false and true nodes
    clear_memos();  // Clear reusable memos before sweeping

    // First, collect all IDs that are preserved
    for (auto it = globals.begin(); it != globals.end();) {
        if (auto value = it->second; std::holds_alternative<Bdd_ptype>(value)) {
            if (const auto& bdd = std::get<Bdd_ptype>(value); bdd.preserved) {
                std::queue<id_type> to_process;
                to_process.push(bdd.id);
                while (!to_process.empty()) {
                    id_type current_id = to_process.front();
                    to_process.pop();

                    if (preserved_ids.contains(current_id)) continue;
                    preserved_ids.insert(current_id);

                    if (const Bdd_Node& node = id_to_iter[current_id]->first;
                        node.type == Bdd_Node::Bdd_type::INTERNAL) {
                        to_process.push(node.high);
                        to_process.push(node.low);
                    }
                }
                ++it;  // preserve this
            } else {
                it = globals.erase(it);  // Remove non-preserved BDDs
            }
        } else {
            ++it;  // Skip non-BDD types
        }
    }

    // Now, remove all non-preserved IDs from the maps
    for (auto it = id_to_iter.begin(); it != id_to_iter.end();) {
        if (!preserved_ids.contains(it->first)) {
            node_to_id.erase(it->second);
            it = id_to_iter.erase(it);
        } else {
            ++it;
        }
    }
}
