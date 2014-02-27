// Copyright (c) 2013 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is a part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
// Author(s) : Dmitry Anisimov, David Bommes, Kai Hormann, and Pierre Alliez.

/*!
  \file Discrete_harmonic_coordinates_2.h
*/

#ifndef CGAL_DISCRETE_HARMONIC_COORDINATES_2_H
#define CGAL_DISCRETE_HARMONIC_COORDINATES_2_H

// Barycentric coordinates headers.
#include <CGAL/Barycentric_coordinates_base_2.h>

// CGAL namespace.
namespace CGAL {

// Barycentric coordinates namespace.
namespace Barycentric_coordinates {

// Examples: See User Manual here - http://doc.cgal.org/latest/Manual/index.html.
// [1] Reference: "M. S. Floater, K. Hormann, and G. Kos. A general construction of barycentric coordinates over convex polygons. Advances in Computational Mathematics, 24(1-4):311-331, 2006.".

/*!
 * \ingroup PkgBarycentric_coordinates_2
 * The class Discrete_harmonic_coordinates_2 implements 2D Discrete Harmonic coordinates ( \cite cgal:bc:fhk-gcbcocp-06, \cite cgal:pp-cdmsc-93, \cite cgal:bc:eddhls-maam-95 )
 * with respect to an arbitrary strictly convex polygon. This class is parameterized by the `CGAL::Polygon_2` and the `Iterator` class.
 * The latter can be any class that fulfills the requirements for an STL iterator. This class is derived
 * from the class `CGAL::Barycentric_coordinates::Barycentric_coordinates_base_2`.
 * For a polygon with three vertices (triangle) it is better to use the class `CGAL::Barycentric_coordinates::Triangle_coordinates_2`.
 * Discrete Harmonic coordinates can be computed exactly. By definition, they do not necesserily give positive values.
 *
 * \sa `Iterator`
 *
 * \pre `type_of_polygon == CGAL::Barycentric_coordinates::STRICTLY_CONVEX`
 */

// This class does not allow the user to use different iterators to output coordinates after
// it has been created. In order to do so, we need to move template parameter Iterator in declaration of the class's functions.
// Can we also use reference in front of iterator like Iterator &output when passing it to the function? 
template<typename Polygon_2, typename Iterator> 
    class Discrete_harmonic_coordinates_2 : public Barycentric_coordinates_base_2<Polygon_2, Iterator>
{

public:

    // Creation.

    /// \name Types
    /// @{

    /// Type of 2D polygon.
    typedef Polygon_2                   Polygon;

    /// Type of the used kernel.
    typedef typename Polygon::Traits    Kernel;

    /// Type of 2D segment.
    typedef typename Polygon::Segment_2 Segment;

    /// Type of 2D point.
    typedef typename Polygon::Point_2   Point;

    /// Number type.
    typedef typename Polygon::FT        Scalar;

    /// Type of the used output iterator.
    typedef Iterator                    OutputIterator;

    /// @}

    /// \name Creation
    /// @{

    /// Creates an instance of the class Discrete_harmonic_coordinates_2 for a provided polygon passed as a reference.
    /// For preconditions and functions to compute weights or coordinates
    /// see the class `CGAL::Barycentric_coordinates::Barycentric_coordinates_base_2`.
    Discrete_harmonic_coordinates_2(const Polygon &_polygon) :
        Barycentric_coordinates_base_2<Polygon, OutputIterator>(_polygon)
    {
        const int number_of_polygon_vertices = _polygon.size();

        // Resize all internal containers.
        r.resize(number_of_polygon_vertices);
        A.resize(number_of_polygon_vertices);
        B.resize(number_of_polygon_vertices);

        weight.resize(number_of_polygon_vertices);
    }

    /// @}

private:

    // Some convenient typedefs.
    typedef Barycentric_coordinates_base_2<Polygon, OutputIterator> Base;
    typedef typename std::vector<Scalar>                   Scalar_vector;

    // Internal global variables.
    Scalar_vector r, A, B, weight;

    Scalar dh_denominator, inverted_dh_denominator;

    // COMPUTATION.

    // Can we somehow use a referenced output: &output?

    // WEIGHTS.

    // Compute Discrete Harmonic weights without normalization. 
    std::pair<OutputIterator, bool> weights(const Point &query_point, OutputIterator output)
    {
        // Get number of vertices in the polygon.
        const int n = Base::number_of_polygon_vertices;

        // Compute areas A, B and distances r following notations from [1]. Split the loop to make computation faster.
        r[0] = CGAL::squared_distance(Base::polygon.vertex(0), query_point);
        A[0] = CGAL::area(Base::polygon.vertex(0)  , Base::polygon.vertex(1), query_point);
        B[0] = CGAL::area(Base::polygon.vertex(n-1), Base::polygon.vertex(1), query_point);

        for(int i = 1; i < n-1; ++i) {
            r[i] = CGAL::squared_distance(Base::polygon.vertex(i), query_point);
            A[i] = CGAL::area(Base::polygon.vertex(i)  , Base::polygon.vertex(i+1), query_point);
            B[i] = CGAL::area(Base::polygon.vertex(i-1), Base::polygon.vertex(i+1), query_point);
        }

        r[n-1] = CGAL::squared_distance(Base::polygon.vertex(n-1), query_point);
        A[n-1] = CGAL::area(Base::polygon.vertex(n-1), Base::polygon.vertex(0), query_point);
        B[n-1] = CGAL::area(Base::polygon.vertex(n-2), Base::polygon.vertex(0), query_point);

        // Compute unnormalized weights following formula (25) with p = 2 from [1].
        CGAL_precondition( A[n-1] != Scalar(0) && A[0] != Scalar(0) );
        *output = (r[1]*A[n-1] - r[0]*B[0] + r[n-1]*A[0]) / (A[n-1] * A[0]);
        ++output;

        for(int i = 1; i < n-1; ++i) {
            CGAL_precondition( A[i-1] != Scalar(0) && A[i] != Scalar(0) );
            *output = (r[i+1]*A[i-1] - r[i]*B[i] + r[i-1]*A[i]) / (A[i-1] * A[i]);
            ++output;
        }

        CGAL_precondition( A[n-2] != Scalar(0) && A[n-1] != Scalar(0) );
        *output = (r[0]*A[n-2] - r[n-1]*B[n-1] + r[n-2]*A[n-1]) / (A[n-2] * A[n-1]);

        // Return weights.
        return std::make_pair(output, true);
    }

    // COORDINATES ON BOUNDED SIDE.

    // Compute Discrete Harmonic coordinates on the bounded side of the polygon with slow O(n^2) but precise algorithm.
    // Here, n - is a number of polygon's vertices.
    std::pair<OutputIterator, bool> coordinates_on_bounded_side_precise(const Point &query_point, OutputIterator output)
    {
        CGAL_precondition( Base::type_of_polygon() == STRICTLY_CONVEX );

        // Get number of vertices in the polygon.
        const int n = Base::number_of_polygon_vertices;

        // Compute areas A, B and distances r following notations from [1]. Split the loop to make computation faster.
        r[0] = CGAL::squared_distance(Base::polygon.vertex(0), query_point);
        A[0] = CGAL::area(Base::polygon.vertex(0)  , Base::polygon.vertex(1), query_point);
        B[0] = CGAL::area(Base::polygon.vertex(n-1), Base::polygon.vertex(1), query_point);

        for(int i = 1; i < n-1; ++i) {
            r[i] = CGAL::squared_distance(Base::polygon.vertex(i), query_point);
            A[i] = CGAL::area(Base::polygon.vertex(i)  , Base::polygon.vertex(i+1), query_point);
            B[i] = CGAL::area(Base::polygon.vertex(i-1), Base::polygon.vertex(i+1), query_point);
        }

        r[n-1] = CGAL::squared_distance(Base::polygon.vertex(n-1), query_point);
        A[n-1] = CGAL::area(Base::polygon.vertex(n-1), Base::polygon.vertex(0), query_point);
        B[n-1] = CGAL::area(Base::polygon.vertex(n-2), Base::polygon.vertex(0), query_point);

        // Initialize weights with numerator of the formula (25) with p = 2 from [1].
        // Then we multiply them by areas A as in the formula (5) in [1]. We also split the loop.
        weight[0] = r[1]*A[n-1] - r[0]*B[0] + r[n-1]*A[0];
        for(int j = 1; j < n-1; ++j) weight[0] *= A[j];

        for(int i = 1; i < n-1; ++i) {
            weight[i] = r[i+1]*A[i-1] - r[i]*B[i] + r[i-1]*A[i];
            for(int j = 0; j < i-1; ++j) weight[i] *= A[j];
            for(int j = i+1; j < n; ++j) weight[i] *= A[j];
        }

        weight[n-1] = r[0]*A[n-2] - r[n-1]*B[n-1] + r[n-2]*A[n-1];
        for(int j = 0; j < n-2; ++j) weight[n-1] *= A[j];

        // Compute the sum of all weights - denominator of Discrete Harmonic coordinates.
        dh_denominator = weight[0];
        for(int i = 1; i < n; ++i) dh_denominator += weight[i];

        // Invert this denominator.
        CGAL_precondition( dh_denominator != Scalar(0) );
        inverted_dh_denominator = Scalar(1) / dh_denominator;

        // Normalize weights and save them as resulting Discrete Harmonic coordinates.
        for(int i = 0; i < n-1; ++i) {
            *output = weight[i] * inverted_dh_denominator;
            ++output;
        }
        *output = weight[n-1] * inverted_dh_denominator;

        // Return coordinates.
        return std::make_pair(output, true);
    }

    // Compute Discrete Harmonic coordinates on the bounded side of the polygon with fast O(n) but less precise algorithm.
    // Here, n - is number of polygon's vertices. Precision is lost next to the boundary.
    // Can we somehow reuse here the computations from the function weights()?
    std::pair<OutputIterator, bool> coordinates_on_bounded_side_fast(const Point &query_point, OutputIterator output)
    {
        CGAL_precondition( Base::type_of_polygon() == STRICTLY_CONVEX );

        // Get number of vertices in the polygon.
        const int n = Base::number_of_polygon_vertices;

        // Compute areas A, B and distances r following notations from [1]. Split the loop to make computation faster.
        r[0] = CGAL::squared_distance(Base::polygon.vertex(0), query_point);
        A[0] = CGAL::area(Base::polygon.vertex(0)  , Base::polygon.vertex(1), query_point);
        B[0] = CGAL::area(Base::polygon.vertex(n-1), Base::polygon.vertex(1), query_point);

        for(int i = 1; i < n-1; ++i) {
            r[i] = CGAL::squared_distance(Base::polygon.vertex(i), query_point);
            A[i] = CGAL::area(Base::polygon.vertex(i)  , Base::polygon.vertex(i+1), query_point);
            B[i] = CGAL::area(Base::polygon.vertex(i-1), Base::polygon.vertex(i+1), query_point);
        }

        r[n-1] = CGAL::squared_distance(Base::polygon.vertex(n-1), query_point);
        A[n-1] = CGAL::area(Base::polygon.vertex(n-1), Base::polygon.vertex(0), query_point);
        B[n-1] = CGAL::area(Base::polygon.vertex(n-2), Base::polygon.vertex(0), query_point);

        // Compute unnormalized weights following formula (25) with p = 2 from [1].
        CGAL_precondition( A[n-1] != Scalar(0) && A[0] != Scalar(0) );
        weight[0] = (r[1]*A[n-1] - r[0]*B[0] + r[n-1]*A[0]) / (A[n-1] * A[0]);

        for(int i = 1; i < n-1; ++i) {
            CGAL_precondition( A[i-1] != Scalar(0) && A[i] != Scalar(0) );
            weight[i] = (r[i+1]*A[i-1] - r[i]*B[i] + r[i-1]*A[i]) / (A[i-1] * A[i]);
        }

        CGAL_precondition( A[n-2] != Scalar(0) && A[n-1] != Scalar(0) );
        weight[n-1] = (r[0]*A[n-2] - r[n-1]*B[n-1] + r[n-2]*A[n-1]) / (A[n-2] * A[n-1]);

        // Compute the sum of all weights - denominator of Discrete Harmonic coordinates.
        dh_denominator = weight[0];
        for(int i = 1; i < n; ++i) dh_denominator += weight[i];

        // Invert this denominator.
        CGAL_precondition( dh_denominator != Scalar(0) );
        inverted_dh_denominator = Scalar(1) / dh_denominator;

        // Normalize weights and save them as resulting Discrete Harmonic coordinates.
        for(int i = 0; i < n-1; ++i) {
            *output = weight[i] * inverted_dh_denominator;
            ++output;
        }
        *output = weight[n-1] * inverted_dh_denominator;

        // Return coordinates.
        return std::make_pair(output, true);
    }

    // COORDINATES ON UNBOUNDED SIDE.

    // Compute Discrete Harmonic coordinates on the unbounded side of the polygon with slow O(n^2) but precise algorithm.
    // Here, n - is a number of polygon's vertices.
    std::pair<OutputIterator, bool> coordinates_on_unbounded_side_precise(const Point &query_point, OutputIterator output)
    {
        std::cout << std::endl << "WARNING: Discrete Harmonic coordinates might be undefined outside a polygon!" << std::endl;

        // Use the same formulas as for bounded side since they are also valid on unbounded side.
        return coordinates_on_bounded_side_precise(query_point, output);
    }

    // Compute Discrete Harmonic coordinates on the unbounded side of the polygon with fast O(n) but less precise algorithm.
    // Here, n - is number of polygon's vertices. Precision is lost next to the boundary.
    std::pair<OutputIterator, bool> coordinates_on_unbounded_side_fast(const Point &query_point, OutputIterator output)
    {
        std::cout << std::endl << "WARNING: Discrete Harmonic coordinates might be undefined outside a polygon!" << std::endl;
        
        // Use the same formulas as for bounded side since they are also valid on unbounded side.
        return coordinates_on_bounded_side_fast(query_point, output);
    }

    // OTHER FUNCTIONS.

    // Print some info about Discrete Harmonic coordinates.
    void print_coordinates_info() const
    {
        std::cout << std::endl << "CONVEXITY: " << std::endl << std::endl;

        const Type_of_polygon type_of_polygon = Base::type_of_polygon();
        if(type_of_polygon == CONCAVE)         std::cout << "Current polygon is not convex. The correct computation is not expected!" << std::endl;
        if(type_of_polygon == WEAKLY_CONVEX)   std::cout << "Current polygon is weakly convex. The correct computation is not expected!" << std::endl;
        if(type_of_polygon == STRICTLY_CONVEX) std::cout << "Current polygon is strictly convex." << std::endl;

        std::cout << std::endl << "TYPE OF COORDINATES: " << std::endl << std::endl;
        std::cout << "Currently computed coordinate functions are Discrete Harmonic coordinates." << std::endl;

        std::cout << std::endl << "INFORMATION ABOUT COORDINATES: " << std::endl << std::endl;
        std::cout << "Discrete Harmonic coordinates are well-defined inside an arbitrary strictly convex polygon, and they can be computed exactly there." << std::endl;

        std::cout << std::endl;
        std::cout << "Inside these polygons they satisfy the following properties: " << std::endl;
        std::cout << "1. Partition of unity or constant precision;" << std::endl;
        std::cout << "2. Homogeneity or linear precision;" << std::endl;
        std::cout << "3. Lagrange property;" << std::endl;
        std::cout << "4. Linearity along edges;" << std::endl;
        std::cout << "5. Smoothness;" << std::endl;
        std::cout << "6. Similarity invariance;" << std::endl;

        std::cout << std::endl;
        std::cout << "For polygons whose vertices lie on a circle, they coincide with Wachspress coordinates and, therefore, inherit all their properties." << std::endl;

        std::cout << std::endl << "REFERENCE: " << std::endl << std::endl;
        std::cout << "M. S. Floater, K. Hormann, and G. Kos. A general construction of barycentric coordinates over convex polygons. Advances in Computational Mathematics, 24(1-4):311-331, 2006." << std::endl;
    }
};

// Class Discrete_harmonic_coordinates_2 with particular std::back_insert_iterator instead of a general one.

/*!
 * \ingroup PkgBarycentric_coordinates_2
 * The class DH_coordinates_2 implements 2D Discrete Harmonic coordinates ( \cite cgal:bc:fhk-gcbcocp-06, \cite cgal:pp-cdmsc-93, \cite cgal:bc:eddhls-maam-95 )
 * with respect to an arbitrary strictly convex polygon. This class is parameterized by the `CGAL::Polygon_2` and a Container class.
 * The latter can be any class that fulfills the requirements for the <a href="http://en.cppreference.com/w/cpp/iterator/back_insert_iterator">`std::back_insert_iterator`</a>. 
 * It defaults to <a href="http://en.cppreference.com/w/cpp/container/vector">`std::vector`</a> container.
 * This class is derived from the class `CGAL::Barycentric_coordinates::Barycentric_coordinates_base_2`.
 * For a polygon with three vertices (triangle) it is better to use the class `CGAL::Barycentric_coordinates::Tri_coordinates_2`.
 * Discrete Harmonic coordinates can be computed exactly. By definition, they do not necesserily give positive values.
 *
 * \sa <a href="http://en.cppreference.com/w/cpp/iterator/back_insert_iterator">`std::back_insert_iterator`</a>
 *
 * \pre `type_of_polygon == CGAL::Barycentric_coordinates::STRICTLY_CONVEX`
 */

template<typename Polygon_2, typename InputContainer = std::vector<typename Polygon_2::FT> > 
    class DH_coordinates_2 : public Discrete_harmonic_coordinates_2<Polygon_2, std::back_insert_iterator<InputContainer> >
{

public:

    // Creation.

    /// \name Types
    /// @{

    /// Type of the used container.
    typedef InputContainer Container;

    /// Type of the base class.
    typedef Discrete_harmonic_coordinates_2<Polygon_2, std::back_insert_iterator<Container> > Base;

    /// @}

    /// \name Creation
    /// @{

    /// Creates an instance of the class Discrete_harmonic_coordinates_2 for a provided polygon passed as a reference.
    /// The used iterator is <a href="http://en.cppreference.com/w/cpp/iterator/back_insert_iterator">`std::back_insert_iterator`</a>.
    DH_coordinates_2(const Polygon_2 &_polygon) : Base(_polygon) { }

    /// @}

    // Weights.

    /// \name Computation of weight functions
    /// @{

    /// Computes Discrete Harmonic weights for any strictly interior query point with respect to all the vertices of the polygon.
    /// \pre `_polygon.bounded_side(query_point) == CGAL::ON_BOUNDED_SIDE`
    inline std::pair<std::back_insert_iterator<Container>, bool> compute_weights(const typename Polygon_2::Point_2 &query_point, Container &container)
    {
        return Base::compute_weights(query_point, std::back_inserter(container));
    }

    /// @}

    // Coordinates.

    /// \name Computation of basis functions at the vertices (with index)
    /// @{

    /// Computes Discrete Harmonic coordinates for a query point, which coincides with one of the polygon's vertices, with known index.
    /// \pre `(0 <= index) && (index < number_of_polygon_vertices)`
    inline std::pair<std::back_insert_iterator<Container>, bool> compute_at_vertex(const int index, Container &container) const
    {
        return Base::compute_at_vertex(index, std::back_inserter(container));
    }

    /// @}

    /// \name Computation of basis functions along edges (with index)
    /// @{

    /// Computes Discrete Harmonic coordinates for a query point on the polygon's boundary with known index of the edge to which this point belongs.
    /// \pre `_polygon.bounded_side(query_point) == CGAL::ON_BOUNDARY`
    /// \pre `(0 <= index) && (index < number_of_polygon_vertices)`
    inline std::pair<std::back_insert_iterator<Container>, bool> compute_on_edge(const typename Polygon_2::Point_2 &query_point, const int index, Container &container) const
    {    
        return Base::compute_on_edge(query_point, index, std::back_inserter(container));
    }

    /// @}

    /// \name Computation of basis functions at an arbitrary point
    /// @{

    /// Computes Discrete Harmonic coordinates for any query point in the plane with respect to all the vertices of the polygon.
    inline std::pair<std::back_insert_iterator<Container>, bool> compute(const typename Polygon_2::Point_2 &query_point, Container &container, Query_point_location query_point_location = UNSPECIFIED_LOCATION, Type_of_algorithm type_of_algorithm = PRECISE)
    {   
        return Base::compute(query_point, std::back_inserter(container), query_point_location, type_of_algorithm);
    }

    /// @}
};

} // namespace Barycentric_coordinates

} // namespace CGAL

#endif // CGAL_DISCRETE_HARMONIC_COORDINATES_2_H