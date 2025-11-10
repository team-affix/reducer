#ifndef LAMBDA_HPP
#define LAMBDA_HPP

#include <cstddef>
#include <map>
#include <memory>
#include <set>

namespace lambda
{

struct expr
{
    using global_map = std::map<size_t, std::unique_ptr<expr>>;
    virtual ~expr() = default;
    virtual std::unique_ptr<expr> lift(size_t a_new_depth) const = 0;
    virtual std::unique_ptr<expr>
    substitute(size_t a_new_depth,
               const std::unique_ptr<expr>& a_arg) const = 0;
    virtual std::unique_ptr<expr> reduce(const global_map& a_globals) const = 0;
    std::unique_ptr<expr> clone() const;
    expr();
    expr(const expr& other) = delete;
    expr& operator=(const expr& other) = delete;
};

struct hole : expr
{
    virtual ~hole() = default;
    std::unique_ptr<expr> lift(size_t a_new_depth) const override;
    std::unique_ptr<expr>
    substitute(size_t a_new_depth,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce(const global_map& a_globals) const override;
    hole(const std::set<size_t>& a_captures);
    std::set<size_t> m_captures;
};

struct func : expr
{
    virtual ~func() = default;
    std::unique_ptr<expr> lift(size_t a_new_depth) const override;
    std::unique_ptr<expr>
    substitute(size_t a_new_depth,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce(const global_map& a_globals) const override;
    func(const std::unique_ptr<expr>& a_body);
    std::unique_ptr<expr> m_body;
};

struct app : expr
{
    virtual ~app() = default;
    std::unique_ptr<expr> lift(size_t a_new_depth) const override;
    std::unique_ptr<expr>
    substitute(size_t a_new_depth,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce(const global_map& a_globals) const override;
    app(const std::unique_ptr<expr>& a_func,
        const std::unique_ptr<expr>& a_arg);
    std::unique_ptr<expr> m_func;
    std::unique_ptr<expr> m_arg;
};

struct local : expr
{
    virtual ~local() = default;
    std::unique_ptr<expr> lift(size_t a_new_depth) const override;
    std::unique_ptr<expr>
    substitute(size_t a_new_depth,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce(const global_map& a_globals) const override;
    local(size_t a_index);
    size_t m_index;
};

struct global : expr
{
    virtual ~global() = default;
    std::unique_ptr<expr> lift(size_t a_new_depth) const override;
    std::unique_ptr<expr>
    substitute(size_t a_new_depth,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce(const global_map& a_globals) const override;
    global(size_t a_index);
    size_t m_index;
};

} // namespace lambda

#endif
