/*
This source file is part of Castor3D (http://castor3d.developpez.com/castor3d.html)
Copyright (c) 2016 dragonjoker59@hotmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef ___C3DGLES3_FRAME_BUFFER_H___
#define ___C3DGLES3_FRAME_BUFFER_H___

#include "GlES3RenderSystemPrerequisites.hpp"

#include "Common/GlES3Bindable.hpp"

#include <FrameBuffer/FrameBuffer.hpp>
#include <FrameBuffer/TextureAttachment.hpp>
#include <FrameBuffer/RenderBufferAttachment.hpp>

namespace GlES3Render
{
	class GlES3FrameBuffer
		: public Castor3D::FrameBuffer
		, private Bindable< std::function< void( int, uint32_t * ) >
			, std::function< void( int, uint32_t const * ) >
			, std::function< void( uint32_t ) > >
	{
		using BindableType = Bindable< std::function< void( int, uint32_t * ) >
			, std::function< void( int, uint32_t const * ) >
			, std::function< void( uint32_t ) > >;

	public:
		/**
		 *\~english
		 *\brief		Constructor.
		 *\para[in]		p_gl		The OpenGL APIs.
		 *\para[in]		p_engine	The engine.
		 *\~french
		 *\brief		Constructeur.
		 *\para[in]		p_gl		Les APIs OpenGL.
		 *\para[in]		p_engine	Le moteur.
		 */
		GlES3FrameBuffer( OpenGlES3 & p_gl, Castor3D::Engine & p_engine );
		/**
		 *\~english
		 *\brief		Destructor.
		 *\~french
		 *\brief		Destructeur.
		 */
		virtual ~GlES3FrameBuffer();
		/**
		 *\copydoc		Castor3D::FrameBuffer::Create
		 */
		bool Create()override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::Destroy
		 */
		void Destroy()override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::SetDrawBuffers
		 */
		void SetDrawBuffers( AttachArray const & p_attaches )const override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::SetReadBuffer
		 */
		void SetReadBuffer( Castor3D::AttachmentPoint p_eAttach, uint8_t p_index )const override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::IsComplete
		 */
		bool IsComplete()const override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::DownloadBuffer
		 */
		void DownloadBuffer( Castor3D::AttachmentPoint p_point, uint8_t p_index, Castor::PxBufferBaseSPtr p_buffer )override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::CreateColourRenderBuffer
		 */
		Castor3D::ColourRenderBufferSPtr CreateColourRenderBuffer( Castor::PixelFormat p_format )override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::CreateDepthStencilRenderBuffer
		 */
		Castor3D::DepthStencilRenderBufferSPtr CreateDepthStencilRenderBuffer( Castor::PixelFormat p_format )override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::CreateAttachment
		 */
		Castor3D::RenderBufferAttachmentSPtr CreateAttachment( Castor3D::RenderBufferSPtr p_renderBuffer )override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::CreateAttachment
		 */
		Castor3D::TextureAttachmentSPtr CreateAttachment( Castor3D::TextureLayoutSPtr p_texture )override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::CreateAttachment
		 */
		Castor3D::TextureAttachmentSPtr CreateAttachment( Castor3D::TextureLayoutSPtr p_texture, Castor3D::CubeMapFace p_face )override;

	public:
		using BindableType::GetGlES3Name;
		using BindableType::GetOpenGlES3;
		using BindableType::IsValid;

	private:
		/**
		 *\copydoc		Castor3D::FrameBuffer::DoBind
		 */
		void DoBind( Castor3D::FrameBufferTarget p_target )const override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::DoUnbind
		 */
		void DoUnbind()const override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::DoBlitInto
		 */
		void DoBlitInto(
			Castor3D::FrameBuffer const & p_buffer,
			Castor::Rectangle const & p_rect,
			Castor::FlagCombination< Castor3D::BufferComponent > const & p_components )const override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::DoStretchInto
		 */
		void DoStretchInto(
			Castor3D::FrameBuffer const & p_buffer,
			Castor::Rectangle const & p_rectSrc,
			Castor::Rectangle const & p_rectDst,
			Castor::FlagCombination< Castor3D::BufferComponent > const & p_components,
			Castor3D::InterpolationMode p_interpolation )const override;
		/**
		 *\copydoc		Castor3D::FrameBuffer::DoClear
		 */
		void DoClear( uint32_t p_targets )override;

	private:
		mutable GlES3FrameBufferMode m_bindingMode;
	};
}

#endif