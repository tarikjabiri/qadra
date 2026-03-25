#ifndef QADRA_RENDER_PASS_HPP
#define QADRA_RENDER_PASS_HPP

#include "gl/Buffer.hpp"
#include "Camera.hpp"
#include "gl/Program.hpp"
#include "gl/VertexArray.hpp"

namespace Qadra::Render
{
  class RenderPass
  {
  public:
    explicit RenderPass ( QString shaderName );

    virtual ~RenderPass () = default;

    RenderPass ( const RenderPass & ) = delete;

    RenderPass &operator= ( const RenderPass & ) = delete;

    void init ();

    template <typename V>
    void upload ( std::span<const V> vertices,
                  GL::Buffer::Usage usage = GL::Buffer::Usage::StaticDraw );

  protected:
    virtual void setupAttributes () = 0;

    void bind () const;

    bool beginRender ( const Core::Camera &camera ) const;

    GL::Buffer m_vbo;
    GL::VertexArray m_vao;
    GL::Program m_program;
    std::size_t m_vertexCount = 0;

  private:
    static QString loadFileContent ( const QString &path );

    QString m_shaderName;
  };

  template <typename V>
  void RenderPass::upload ( std::span<const V> vertices, GL::Buffer::Usage usage )
  {
    if ( vertices.empty () )
    {
      m_vertexCount = 0;
      return;
    }

    m_vbo.allocate ( vertices, usage );

    m_vao.attachVertexBuffer ( 0, m_vbo, 0, sizeof ( V ) );

    m_vertexCount = vertices.size ();
  }
} // namespace Qadra::Render

#endif // QADRA_RENDER_PASS_HPP
