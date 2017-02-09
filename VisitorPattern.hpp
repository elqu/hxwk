#define ABSTR_ACCEPT(visitor_type) \
    virtual void accept(visitor_type& visitor) = 0

#define ACCEPT(visitor_type) \
    void accept(visitor_type& visitor) override {visitor.visit(*this);}

#define ABSTR_VISIT(visitable_type) \
    virtual void visit(visitable_type& visitable) = 0

#define VISIT(visitable_type) \
    void visit(visitable_type& visitable) override
