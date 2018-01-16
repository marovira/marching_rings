#include "athena/blob/Bsoid.hpp"

namespace athena
{
    namespace blob
    {
        Bsoid::Bsoid() :
            mName("model")
        { }

        Bsoid::Bsoid(tree::BlobTree const& model, std::string const& name) :
            mModel(model),
            mName(name)
        { }

        void Bsoid::setModel(tree::BlobTree const& model)
        {
            mModel = model;
        }

        void Bsoid::setName(std::string const& name)
        {
            mName = name;
        }

        void Bsoid::polygonize()
        {

        }

        void Bsoid::saveCubicLattice() const
        {

        }
    }
}