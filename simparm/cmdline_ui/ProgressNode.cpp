
    connections.push_back( a.notify_on_value_change( boost::bind( &Node::print_attribute_value, this, boost::cref(a) ) ) );
