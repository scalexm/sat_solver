//
//  conflict_graph.hpp
//  solver
//
//  Created by Alexandre Martin on 30/03/2016.
//  Copyright © 2016 scalexm. All rights reserved.
//

#ifndef CONFLICT_GRAPH_HPP
#define CONFLICT_GRAPH_HPP

#include <boost/variant.hpp>
#include <vector>

namespace detail {
    struct conflict { };
    using conflict_node = boost::variant<conflict, int>;
    using conflict_graph = std::vector<std::vector<conflict_node>>;
}

#endif
