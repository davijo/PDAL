/******************************************************************************
* Copyright (c) 2013, Andrew Bell (andrew.bell.ia@gmail.com)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include <pdal/filters/HexBin.hpp>

#include <boost/concept_check.hpp> // ignore_unused_variable_warning

#include <pdal/PointBuffer.hpp>
#include <pdal/GlobalEnvironment.hpp>



namespace pdal
{
namespace filters
{




HexBin::HexBin(Stage& prevStage, const Options& options)
    : pdal::Filter(prevStage, options)
{

    return;
}

HexBin::~HexBin()
{
    
}
void HexBin::initialize()
{
    Filter::initialize();
    return;
}




Options HexBin::getDefaultOptions()
{
    Options options;
    return options;
}

pdal::StageSequentialIterator* HexBin::createSequentialIterator(PointBuffer& buffer) const
{
    return new pdal::filters::iterators::sequential::HexBin(*this, buffer);
}

// pdal::StageRandomIterator* HexBin::createRandomIterator(PointBuffer& buffer) const
// {
//     return new pdal::filters::iterators::random::HexBin(*this, buffer);
// }


namespace iterators
{

namespace hexbin
{
    
IteratorBase::IteratorBase( pdal::filters::HexBin const& filter, 
                            PointBuffer& buffer)
: m_filter(filter)
{
    Schema const& schema = buffer.getSchema();
    m_dim_x = &schema.getDimension("X");
    m_dim_y = &schema.getDimension("Y");
}



} // hexbin

namespace sequential
{


HexBin::HexBin(const pdal::filters::HexBin& filter, PointBuffer& buffer)
    : pdal::FilterSequentialIterator(filter, buffer)
    , hexbin::IteratorBase(filter, buffer)
{
    return;
}

boost::uint32_t HexBin::readBufferImpl(PointBuffer& buffer)
{
    
    const boost::uint32_t numPoints = getPrevIterator().read(buffer);
    
    
    for (boost::uint32_t i = 0; i < buffer.getNumPoints(); ++i)
    {
        int32_t xi = buffer.getField<int32_t>(*m_dim_x, i);
        int32_t yi = buffer.getField<int32_t>(*m_dim_y, i);
        double x = m_dim_x->applyScaling<int32_t>(xi);
        double y = m_dim_y->applyScaling<int32_t>(yi);
#ifdef PDAL_HAVE_HEXER

        m_samples.push_back(hexer::Point(x,y));
#endif
    }
        
    return numPoints;
}

void HexBin::readBufferEndImpl(PointBuffer&)
{

    // double hexsize = hexer::computeHexSize(samples);
    double hexsize =5 ;
    int dense_limit = 10;

#ifdef PDAL_HAVE_HEXER

    m_grid = new hexer::HexGrid(hexsize, dense_limit);
    for (std::vector<hexer::Point>::size_type i = 0; i < m_samples.size(); ++i)
    {
        m_grid->addPoint(m_samples[i]);
    }


    m_grid->findShapes();
#endif
        
}
    

boost::uint64_t HexBin::skipImpl(boost::uint64_t count)
{
    getPrevIterator().skip(count);
    return count;
}


bool HexBin::atEndImpl() const
{
    return getPrevIterator().atEnd();
}

} // sequential

// namespace random
// {
// 
// 
// HexBin::HexBin(const pdal::filters::HexBin& filter, PointBuffer& buffer)
//     : pdal::FilterRandomIterator(filter, buffer)
//     , hexbin::IteratorBase(filter, buffer)
// {
//     return;
// }
// 
// boost::uint32_t HexBin::readBufferImpl(PointBuffer& buffer)
// {
//     
//     pdal::StageRandomIterator& iterator = getPrevIterator();
//     
//     const boost::uint32_t numPoints = iterator.read(buffer);
//     
//     return numPoints;
// }
// 
// 
// boost::uint64_t HexBin::seekImpl(boost::uint64_t count)
// {
//     
//     return getPrevIterator().seek(count);
// }
// 
// } // random



} // filters
} // pdal
} // namespaces
