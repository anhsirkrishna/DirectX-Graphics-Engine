#include "Drawable.h"
#include "IndexBuffer.h"
#include <cassert>

/*
* Binds all the required bindables (eg. Shaders, CBuffers)
* Calls gfx.DrawIndex()
*/
void Drawable::Draw(Graphics& gfx) const {
	for (auto& b : binds)
	{
		b->Bind(gfx);
	}
	for (auto& b : GetStaticBinds())
	{
		b->Bind(gfx);
	}
	gfx.DrawIndexed(pIndexBuffer->GetCount());
}

/*
* Adds a Bindable to the list of Binds
*/
void Drawable::AddBind(std::unique_ptr<Bindable> bind) {
	assert("*Must* use AddIndexBuffer to bind index buffer" && typeid(*bind) != typeid(IndexBuffer));
	binds.push_back(std::move(bind));
}

/*
* Adds an index buffer to the list of binds.
*/
void Drawable::AddIndexBuffer(std::unique_ptr<class IndexBuffer> ibuf) {
	assert("Attempting to add index buffer a second time" && pIndexBuffer == nullptr);
	pIndexBuffer = ibuf.get();
	binds.push_back(std::move(ibuf));
}
