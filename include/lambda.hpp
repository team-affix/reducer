#ifndef LAMBDA_HPP
#define LAMBDA_HPP

#include <cstddef>
#include <memory>
#include <set>

namespace lambda
{

struct expr
{
    virtual ~expr() = default;
    virtual std::unique_ptr<expr> lift(size_t a_new_depth) const = 0;
    virtual std::unique_ptr<expr>
    substitute(size_t a_new_depth,
               const std::unique_ptr<expr>& a_arg) const = 0;
    virtual std::unique_ptr<expr> reduce() const = 0;
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
    std::unique_ptr<expr> reduce() const override;
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
    std::unique_ptr<expr> reduce() const override;
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
    std::unique_ptr<expr> reduce() const override;
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
    std::unique_ptr<expr> reduce() const override;
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
    std::unique_ptr<expr> reduce() const override;
    global(size_t a_index);
    size_t m_index;
};

} // namespace lambda

#endif
