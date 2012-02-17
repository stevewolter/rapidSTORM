
template <typename PixelType, int Dimensions>
samplepos Traits< dStorm::Image<PixelType,Dimensions> >
::size_in_sample_space() const
{
    typename ImageTraits<Dimensions>::Size size = this->size;
    size.array() -= 1 * camera::pixel;
    samplepos p = samplepos::Constant(0.0f * si::meter);
    for (int pl = 0; pl < this->plane_count(); ++pl) {
        traits::Projection::SamplePosition xy
            = this->plane(pl).projection()->pixel_in_sample_space( 
                                         size.template head<2>() );
        p.x() = std::max( p.x(), xy.x() );
        p.y() = std::max( p.y(), xy.y() );
        p.z() = std::max( p.z(), 
                          samplepos::Scalar(*this->plane(pl).z_position) );
    }
    return p;
}
