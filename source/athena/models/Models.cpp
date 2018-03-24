#include "athena/models/Models.hpp"

#include "athena/fields/Sphere.hpp"
#include "athena/fields/Cylinder.hpp"
#include "athena/fields/Cone.hpp"
#include "athena/fields/Torus.hpp"

#include "athena/operators/Blend.hpp"

#include "athena/tree/BlobTree.hpp"
#include "athena/polygonizer/Bsoid.hpp"

#include <tuple>

namespace athena
{
    namespace models
    {
        // Global usings.
        using fields::ImplicitFieldPtr;
        using operators::ImplicitOperatorPtr;
        using tree::BlobTree;
        using polygonizer::Bsoid;
        using polygonizer::SlicingAxes;

        using Resolution = std::tuple<std::size_t, std::size_t, std::size_t>;

        constexpr Resolution lowResolution = { 16, 8, 4 };
        constexpr Resolution midResolution = { 32, 16, 8 };
        constexpr Resolution highResolution = { 64, 32, 8 };

        constexpr Resolution currentResolution = lowResolution;

        polygonizer::Bsoid makeSphere()
        {
            using fields::Sphere;

            ImplicitFieldPtr sphere = std::make_shared<Sphere>();
            BlobTree tree;
            tree.insertField(sphere);
            tree.insertNodeTree({ { -1 } });
            tree.insertFieldTree(sphere);

            Bsoid soid(tree, "sphere");
            soid.setSlicingAxis(SlicingAxes::YAxis);
            soid.setNumCrossSections(std::get<0>(currentResolution));
            soid.makeCrossSections(std::get<1>(currentResolution),
                std::get<2>(currentResolution));
            return soid;
        }

        polygonizer::Bsoid makePeanut()
        {
            using atlas::math::Point;
            using fields::Sphere;
            using operators::Blend;

            ImplicitFieldPtr sphere1 =
                std::make_shared<Sphere>(1.0f, Point(1.0f, 0, 0));
            ImplicitFieldPtr sphere2 =
                std::make_shared<Sphere>(1.0f, Point(-1.0f, 0, 0));
            ImplicitOperatorPtr blend = std::make_shared<Blend>();
            blend->insertFields({ sphere1, sphere2 });

            BlobTree tree;
            tree.insertFields({ sphere1, sphere2, blend });
            tree.insertNodeTree({ { -1 }, { -1 }, { 0, 1 } });
            tree.insertFieldTree(blend);

            Bsoid soid(tree, "peanut");
            soid.setSlicingAxis(SlicingAxes::YAxis);
            soid.setNumCrossSections(std::get<0>(currentResolution));
            soid.makeCrossSections(std::get<1>(currentResolution),
                std::get<2>(currentResolution));
            return soid;

        }

        polygonizer::Bsoid makeCylinder()
        {
            using atlas::math::Point;
            using fields::Cylinder;

            ImplicitFieldPtr cylinder = std::make_shared<Cylinder>();
            BlobTree tree;
            tree.insertField(cylinder);
            tree.insertNodeTree({ { -1 } });
            tree.insertFieldTree(cylinder);

            Bsoid soid(tree, "cylinder");
            soid.setSlicingAxis(SlicingAxes::ZAxis);
            soid.setNumCrossSections(std::get<0>(currentResolution));
            soid.makeCrossSections(std::get<1>(currentResolution),
                std::get<2>(currentResolution));
            return soid;
        }

        polygonizer::Bsoid makeCone()
        {
            using atlas::math::Point;
            using fields::Cone;

            ImplicitFieldPtr cone = std::make_shared<Cone>();
            BlobTree tree;
            tree.insertField(cone);
            tree.insertNodeTree({ { -1 } });
            tree.insertFieldTree(cone);

            Bsoid soid(tree, "cone");
            soid.setSlicingAxis(SlicingAxes::ZAxis);
            soid.setNumCrossSections(std::get<0>(currentResolution));
            soid.makeCrossSections(std::get<1>(currentResolution),
                std::get<2>(currentResolution));
            return soid;
        }

        polygonizer::Bsoid makeTorus()
        {
            using atlas::math::Point;
            using fields::Torus;

            ImplicitFieldPtr torus = std::make_shared<Torus>();
            BlobTree tree;
            tree.insertField(torus);
            tree.insertNodeTree({ { -1 } });
            tree.insertFieldTree(torus);

            Bsoid soid(tree, "torus");
            soid.setSlicingAxis(SlicingAxes::ZAxis);
            soid.setNumCrossSections(std::get<0>(currentResolution));
            soid.makeCrossSections(std::get<1>(currentResolution),
                std::get<2>(currentResolution));
            return soid;
        }
    }
}