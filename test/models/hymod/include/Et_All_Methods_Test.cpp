#include "gtest/gtest.h"
#include <stdio.h>
#include "kernels/evapotranspiration/EtWrapperFunction.hpp"

class EtCalcKernelTest : public ::testing::Test {

    protected:

    EtCalcKernelTest() {

    }

    ~EtCalcKernelTest() override {

    }

    void SetUp() override;

    void TearDown() override;

    void setupArbitraryExampleCase();

};

void EtCalcKernelTest::SetUp() {
    setupArbitraryExampleCase();
}

void EtCalcKernelTest::TearDown() {

}

void EtCalcKernelTest::setupArbitraryExampleCase() {

}


TEST_F(EtCalcKernelTest, TestEnergyBalanceMethod)
{
  struct evapotranspiration_options set_et_options;
  double et_m_per_s;
  int et_method_option;


  // FLAGS
  int yes_aorc; // if TRUE then using AORC forcing data- if FALSE then we must calculate incoming short/longwave rad.
  int yes_wrf;  // if TRUE then we get radiation winds etc. from WRF output.  TODO not implemented.

  struct aorc_forcing_data aorc;

  struct evapotranspiration_options et_options;
  struct evapotranspiration_params  et_params;
  struct evapotranspiration_forcing et_forcing;
  struct intermediate_vars inter_vars;

  struct surface_radiation_params   surf_rad_params;
  struct surface_radiation_forcing  surf_rad_forcing;

  struct solar_radiation_options    solar_options; 
  struct solar_radiation_parameters solar_params;
  struct solar_radiation_forcing    solar_forcing;
  struct solar_radiation_results    solar_results;


  double wind_speed_at_2_m;
  double et_mm_per_d;
  double numerator,denominator;
  double saturation_vapor_pressure_Pa;
  double actual_vapor_pressure_Pa;

  //###################################################################################################
  // THE VALUE OF THESE FLAGS DETERMINE HOW THIS CODE BEHAVES.  CYCLE THROUGH THESE FOR THE UNIT TEST.
  //###################################################################################################
  // Set this flag to TRUE if meteorological inputs come from AORC
  set_et_options.yes_aorc = TRUE;                      // if TRUE, it means that we are using AORC data.
  set_et_options.shortwave_radiation_provided = FALSE;
  et_options.yes_aorc = set_et_options.yes_aorc;
  et_options.shortwave_radiation_provided = set_et_options.shortwave_radiation_provided;


  // -----UNIT TEST RESULTS:-----

  // energy balance method:
  // calculated instantaneous potential evapotranspiration (PET) =8.594743e-08 m/s
  // calculated instantaneous potential evapotranspiration (PET) =7.425858 mm/d


  //###################################################################################################
  // MAKE UP SOME TYPICAL AORC DATA.  THESE VALUES DRIVE THE UNIT TESTS.
  //###################################################################################################
  //read_aorc_data().  TODO: These data come from some aorc reading/parsing function.
  //---------------------------------------------------------------------------------------------------------------

  aorc.incoming_longwave_W_per_m2     =  117.1;
  aorc.incoming_shortwave_W_per_m2    =  599.7;
  aorc.surface_pressure_Pa            =  101300.0;
  aorc.specific_humidity_2m_kg_per_kg =  0.00778;      // results in a relative humidity of 40%
  aorc.air_temperature_2m_K           =  25.0+TK;
  aorc.u_wind_speed_10m_m_per_s       =  1.54;
  aorc.v_wind_speed_10m_m_per_s       =  3.2;
  aorc.latitude                       =  37.865211;
  aorc.longitude                      =  -98.12345;
  aorc.time                           =  111111112;


  // populate the evapotranspiration forcing data structure:
  // this part of code does not explicitly setting values, moved to et_wrapper_function()


  // ET forcing values that come from somewhere else...
  //---------------------------------------------------------------------------------------------------------------
  et_forcing.canopy_resistance_sec_per_m   = 50.0; // TODO: from plant growth model
  et_forcing.water_temperature_C           = 15.5; // TODO: from soil or lake thermal model
  et_forcing.ground_heat_flux_W_per_sq_m=-10.0;    // TODO from soil thermal model.  Negative denotes downward.

  if(et_options.yes_aorc==TRUE)
  {
    et_params.wind_speed_measurement_height_m=10.0;  // AORC uses 10m.  Must convert to wind speed at 2 m height.
  }    
  et_params.humidity_measurement_height_m=2.0; 
  et_params.vegetation_height_m=0.12;   // used for unit test of aerodynamic resistance used in Penman Monteith method.     
  et_params.zero_plane_displacement_height_m=0.0003;  // 0.03 cm for unit testing
  et_params.momentum_transfer_roughness_length_m=0.0;  // zero means that default values will be used in routine.
  et_params.heat_transfer_roughness_length_m=0.0;      // zero means that default values will be used in routine.


  // surface radiation parameter values that are a function of land cover.   Must be assigned from land cover type.
  //---------------------------------------------------------------------------------------------------------------
  surf_rad_params.surface_longwave_emissivity=1.0; // this is 1.0 for granular surfaces, maybe 0.97 for water
  surf_rad_params.surface_shortwave_albedo=0.22;  // this is a function of solar elev. angle for most surfaces.   


  if(et_options.yes_aorc!=TRUE)
  {
    // these values are needed if we don't have incoming longwave radiation measurements.
    surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
  }

    // these values are needed if we don't have incoming longwave radiation measurements.
    surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
    

  // Surface radiation forcing parameter values that must come from other models
  //---------------------------------------------------------------------------------------------------------------
  surf_rad_forcing.surface_skin_temperature_C = 12.0;  // TODO from soil thermal model or vegetation model.

  if(et_options.shortwave_radiation_provided=FALSE)
  {
    // populate the elements of the structures needed to calculate shortwave (solar) radiation, and calculate it
    // ### OPTIONS ###
    solar_options.cloud_base_height_known=FALSE;  // set to TRUE if the solar_forcing.cloud_base_height_m is known.

    // ### PARAMS ###
    solar_params.latitude_degrees      =  37.25;   // THESE VALUES ARE FOR THE UNIT TEST
    solar_params.longitude_degrees     = -97.5554; // THESE VALUES ARE FOR THE UNIT TEST
    solar_params.site_elevation_m      = 303.333;  // THESE VALUES ARE FOR THE UNIT TEST  

    // ### FORCING ###
    solar_forcing.cloud_cover_fraction         =   0.5;   // THESE VALUES ARE FOR THE UNIT TEST 
    solar_forcing.atmospheric_turbidity_factor =   2.0;   // 2.0 = clear mountain air, 5.0= smoggy air
    solar_forcing.day_of_year                  =  208;    // THESE VALUES ARE FOR THE UNIT TEST
    solar_forcing.zulu_time_h                  =  20.567; // THESE VALUES ARE FOR THE UNIT TEST

    // UNIT TEST RESULTS
    // CALCULATED SOLAR FLUXES
    // at time:     20.56700000 UTC
    // at site latitude: 37.250000 deg. longitude:-97.555400 deg.  elevation:303.333000 m
    // Shortwave radiation clear-sky flux calculations:
    // -above canopy/snow perpendicular to Earth-Sun line is      =964.56166277 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =661.40396086 W/m2
    // Shortwave radiation clear-sky flux calculations with 0.5000 cloud cover fraction:
    // -above canopy/snow perpendicular to Earth-Sun line is      =807.82039257 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =553.92581722 W/m2
    // CALCULATED ANGLES DESCRIBING VECTOR POINTING TO THE SUN
    // solar elevation angle:     43.29101185 degrees
    // solar azimuth:            225.06371958 degrees
    // local hour angle:          31.01549773 degrees
    // Number of tests passed=7 of 7.
    // UNIT TEST PASSED.
  }

  //et_method_option = 1;    use_energy_balance_method
  //et_method_option = 2;    use_aerodynamic_method
  //et_method_option = 3;    use_combination_method
  //et_method_option = 4;    use_priestley_taylor_method
  //et_method_option = 5;    use_penman_monteith_method
  et_method_option = 1;

  //std::cout << "Input et_method_option: 1-5" << std::endl;
  //std::cin >> et_method_option;

  //the following two variables are set in function et_function_calc_wrapper()
  //set_et_options->yes_aorc=TRUE;
  //set_et_options->shortwave_radiation_provided=FALSE;
  set_et_options.use_energy_balance_method   = FALSE;
  set_et_options.use_aerodynamic_method      = FALSE;
  set_et_options.use_combination_method      = FALSE;
  set_et_options.use_priestley_taylor_method = FALSE;
  set_et_options.use_penman_monteith_method  = FALSE;


  if (et_method_option == 1)
    set_et_options.use_energy_balance_method   = TRUE;
  if (et_method_option == 2)
    set_et_options.use_aerodynamic_method      = TRUE;
  if (et_method_option == 3)
    set_et_options.use_combination_method      = TRUE;
  if (et_method_option == 4)
    set_et_options.use_priestley_taylor_method = TRUE;
  if (et_method_option == 5)
    set_et_options.use_penman_monteith_method  = TRUE;

  et_m_per_s = et_wrapper_function(&aorc, &solar_options, &solar_params, &solar_forcing,
                                   &set_et_options, &et_params, &et_forcing, 
                                   &surf_rad_params, &surf_rad_forcing);

  printf("In main: calculated instantaneous potential evapotranspiration (PET) =%8.6e m/s\n",et_m_per_s);

  //EXPECT_DOUBLE_EQ (8.594743e-08, et_m_per_s);
  EXPECT_LT(abs(et_m_per_s-8.594743e-08), 1.0e-08);
  ASSERT_TRUE(true);

}


TEST_F(EtCalcKernelTest, TestAerodynamicMethod)
{
  struct evapotranspiration_options set_et_options;
  double et_m_per_s;
  int et_method_option;


  // FLAGS
  int yes_aorc; // if TRUE then using AORC forcing data- if FALSE then we must calculate incoming short/longwave rad.
  int yes_wrf;  // if TRUE then we get radiation winds etc. from WRF output.  TODO not implemented.

  struct aorc_forcing_data aorc;

  struct evapotranspiration_options et_options;
  struct evapotranspiration_params  et_params;
  struct evapotranspiration_forcing et_forcing;
  struct intermediate_vars inter_vars;

  struct surface_radiation_params   surf_rad_params;
  struct surface_radiation_forcing  surf_rad_forcing;

  struct solar_radiation_options    solar_options; 
  struct solar_radiation_parameters solar_params;
  struct solar_radiation_forcing    solar_forcing;
  struct solar_radiation_results    solar_results;


  double wind_speed_at_2_m;
  double et_mm_per_d;
  double numerator,denominator;
  double saturation_vapor_pressure_Pa;
  double actual_vapor_pressure_Pa;

  //###################################################################################################
  // THE VALUE OF THESE FLAGS DETERMINE HOW THIS CODE BEHAVES.  CYCLE THROUGH THESE FOR THE UNIT TEST.
  //###################################################################################################
  // Set this flag to TRUE if meteorological inputs come from AORC
  set_et_options.yes_aorc = TRUE;                      // if TRUE, it means that we are using AORC data.
  set_et_options.shortwave_radiation_provided = FALSE;
  et_options.yes_aorc = set_et_options.yes_aorc;
  et_options.shortwave_radiation_provided = set_et_options.shortwave_radiation_provided;


  // -----UNIT TEST RESULTS:-----

  // -----aerodynamic method:-----
  // calculated instantaneous potential evapotranspiration (PET) =8.977490e-08 m/s
  // calculated instantaneous potential evapotranspiration (PET) =7.756551 mm/d
  

  //###################################################################################################
  // MAKE UP SOME TYPICAL AORC DATA.  THESE VALUES DRIVE THE UNIT TESTS.
  //###################################################################################################
  //read_aorc_data().  TODO: These data come from some aorc reading/parsing function.
  //---------------------------------------------------------------------------------------------------------------

  aorc.incoming_longwave_W_per_m2     =  117.1;
  aorc.incoming_shortwave_W_per_m2    =  599.7;
  aorc.surface_pressure_Pa            =  101300.0;
  aorc.specific_humidity_2m_kg_per_kg =  0.00778;      // results in a relative humidity of 40%
  aorc.air_temperature_2m_K           =  25.0+TK;
  aorc.u_wind_speed_10m_m_per_s       =  1.54;
  aorc.v_wind_speed_10m_m_per_s       =  3.2;
  aorc.latitude                       =  37.865211;
  aorc.longitude                      =  -98.12345;
  aorc.time                           =  111111112;


  // populate the evapotranspiration forcing data structure:
  // this part of code does not explicitly setting values, moved to et_wrapper_function()


  // ET forcing values that come from somewhere else...
  //---------------------------------------------------------------------------------------------------------------
  et_forcing.canopy_resistance_sec_per_m   = 50.0; // TODO: from plant growth model
  et_forcing.water_temperature_C           = 15.5; // TODO: from soil or lake thermal model
  et_forcing.ground_heat_flux_W_per_sq_m=-10.0;    // TODO from soil thermal model.  Negative denotes downward.

  if(et_options.yes_aorc==TRUE)
  {
    et_params.wind_speed_measurement_height_m=10.0;  // AORC uses 10m.  Must convert to wind speed at 2 m height.
  }    
  et_params.humidity_measurement_height_m=2.0; 
  et_params.vegetation_height_m=0.12;   // used for unit test of aerodynamic resistance used in Penman Monteith method.     
  et_params.zero_plane_displacement_height_m=0.0003;  // 0.03 cm for unit testing
  et_params.momentum_transfer_roughness_length_m=0.0;  // zero means that default values will be used in routine.
  et_params.heat_transfer_roughness_length_m=0.0;      // zero means that default values will be used in routine.


  // surface radiation parameter values that are a function of land cover.   Must be assigned from land cover type.
  //---------------------------------------------------------------------------------------------------------------
  surf_rad_params.surface_longwave_emissivity=1.0; // this is 1.0 for granular surfaces, maybe 0.97 for water
  surf_rad_params.surface_shortwave_albedo=0.22;  // this is a function of solar elev. angle for most surfaces.   


  if(et_options.yes_aorc!=TRUE)
  {
    // these values are needed if we don't have incoming longwave radiation measurements.
    surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
  }

    // these values are needed if we don't have incoming longwave radiation measurements.
    surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
    

  // Surface radiation forcing parameter values that must come from other models
  //---------------------------------------------------------------------------------------------------------------
  surf_rad_forcing.surface_skin_temperature_C = 12.0;  // TODO from soil thermal model or vegetation model.

  if(et_options.shortwave_radiation_provided=FALSE)
  {
    // populate the elements of the structures needed to calculate shortwave (solar) radiation, and calculate it
    // ### OPTIONS ###
    solar_options.cloud_base_height_known=FALSE;  // set to TRUE if the solar_forcing.cloud_base_height_m is known.

    // ### PARAMS ###
    solar_params.latitude_degrees      =  37.25;   // THESE VALUES ARE FOR THE UNIT TEST
    solar_params.longitude_degrees     = -97.5554; // THESE VALUES ARE FOR THE UNIT TEST
    solar_params.site_elevation_m      = 303.333;  // THESE VALUES ARE FOR THE UNIT TEST  

    // ### FORCING ###
    solar_forcing.cloud_cover_fraction         =   0.5;   // THESE VALUES ARE FOR THE UNIT TEST 
    solar_forcing.atmospheric_turbidity_factor =   2.0;   // 2.0 = clear mountain air, 5.0= smoggy air
    solar_forcing.day_of_year                  =  208;    // THESE VALUES ARE FOR THE UNIT TEST
    solar_forcing.zulu_time_h                  =  20.567; // THESE VALUES ARE FOR THE UNIT TEST

    // UNIT TEST RESULTS
    // CALCULATED SOLAR FLUXES
    // at time:     20.56700000 UTC
    // at site latitude: 37.250000 deg. longitude:-97.555400 deg.  elevation:303.333000 m
    // Shortwave radiation clear-sky flux calculations:
    // -above canopy/snow perpendicular to Earth-Sun line is      =964.56166277 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =661.40396086 W/m2
    // Shortwave radiation clear-sky flux calculations with 0.5000 cloud cover fraction:
    // -above canopy/snow perpendicular to Earth-Sun line is      =807.82039257 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =553.92581722 W/m2
    // CALCULATED ANGLES DESCRIBING VECTOR POINTING TO THE SUN
    // solar elevation angle:     43.29101185 degrees
    // solar azimuth:            225.06371958 degrees
    // local hour angle:          31.01549773 degrees
    // Number of tests passed=7 of 7.
    // UNIT TEST PASSED.
  }

  //et_method_option = 1;    use_energy_balance_method
  //et_method_option = 2;    use_aerodynamic_method
  //et_method_option = 3;    use_combination_method
  //et_method_option = 4;    use_priestley_taylor_method
  //et_method_option = 5;    use_penman_monteith_method
  et_method_option = 2;

  //std::cout << "Input et_method_option: 1-5" << std::endl;
  //std::cin >> et_method_option;

  //the following two variables are set in function et_function_calc_wrapper()
  //set_et_options->yes_aorc=TRUE;
  //set_et_options->shortwave_radiation_provided=FALSE;
  set_et_options.use_energy_balance_method   = FALSE;
  set_et_options.use_aerodynamic_method      = FALSE;
  set_et_options.use_combination_method      = FALSE;
  set_et_options.use_priestley_taylor_method = FALSE;
  set_et_options.use_penman_monteith_method  = FALSE;

  if (et_method_option == 1)
    set_et_options.use_energy_balance_method   = TRUE;
  if (et_method_option == 2)
    set_et_options.use_aerodynamic_method      = TRUE;
  if (et_method_option == 3)
    set_et_options.use_combination_method      = TRUE;
  if (et_method_option == 4)
    set_et_options.use_priestley_taylor_method = TRUE;
  if (et_method_option == 5)
    set_et_options.use_penman_monteith_method  = TRUE;

  et_m_per_s = et_wrapper_function(&aorc, &solar_options, &solar_params, &solar_forcing,
                                   &set_et_options, &et_params, &et_forcing, 
                                   &surf_rad_params, &surf_rad_forcing);

  printf("In main: calculated instantaneous potential evapotranspiration (PET) =%8.6e m/s\n",et_m_per_s);

  //EXPECT_DOUBLE_EQ (8.977490e-08, et_m_per_s);
  EXPECT_LT(abs(et_m_per_s-8.977490e-08), 1.0e-08);
  ASSERT_TRUE(true);

}


TEST_F(EtCalcKernelTest, TestCombinationMethod)
{
  struct evapotranspiration_options set_et_options;
  double et_m_per_s;
  int et_method_option;


  // FLAGS
  int yes_aorc; // if TRUE then using AORC forcing data- if FALSE then we must calculate incoming short/longwave rad.
  int yes_wrf;  // if TRUE then we get radiation winds etc. from WRF output.  TODO not implemented.

  struct aorc_forcing_data aorc;

  struct evapotranspiration_options et_options;
  struct evapotranspiration_params  et_params;
  struct evapotranspiration_forcing et_forcing;
  struct intermediate_vars inter_vars;

  struct surface_radiation_params   surf_rad_params;
  struct surface_radiation_forcing  surf_rad_forcing;

  struct solar_radiation_options    solar_options; 
  struct solar_radiation_parameters solar_params;
  struct solar_radiation_forcing    solar_forcing;
  struct solar_radiation_results    solar_results;


  double wind_speed_at_2_m;
  double et_mm_per_d;
  double numerator,denominator;
  double saturation_vapor_pressure_Pa;
  double actual_vapor_pressure_Pa;

  //###################################################################################################
  // THE VALUE OF THESE FLAGS DETERMINE HOW THIS CODE BEHAVES.  CYCLE THROUGH THESE FOR THE UNIT TEST.
  //###################################################################################################
  // Set this flag to TRUE if meteorological inputs come from AORC
  set_et_options.yes_aorc = TRUE;                      // if TRUE, it means that we are using AORC data.
  set_et_options.shortwave_radiation_provided = FALSE;
  et_options.yes_aorc = set_et_options.yes_aorc;
  et_options.shortwave_radiation_provided = set_et_options.shortwave_radiation_provided;


  // -----UNIT TEST RESULTS:-----

  // -----combination method:-----
  // calculated instantaneous potential evapotranspiration (PET) =8.694909e-08 m/s
  // calculated instantaneous potential evapotranspiration (PET) =7.512402 mm/d
  

  //###################################################################################################
  // MAKE UP SOME TYPICAL AORC DATA.  THESE VALUES DRIVE THE UNIT TESTS.
  //###################################################################################################
  //read_aorc_data().  TODO: These data come from some aorc reading/parsing function.
  //---------------------------------------------------------------------------------------------------------------

  aorc.incoming_longwave_W_per_m2     =  117.1;
  aorc.incoming_shortwave_W_per_m2    =  599.7;
  aorc.surface_pressure_Pa            =  101300.0;
  aorc.specific_humidity_2m_kg_per_kg =  0.00778;      // results in a relative humidity of 40%
  aorc.air_temperature_2m_K           =  25.0+TK;
  aorc.u_wind_speed_10m_m_per_s       =  1.54;
  aorc.v_wind_speed_10m_m_per_s       =  3.2;
  aorc.latitude                       =  37.865211;
  aorc.longitude                      =  -98.12345;
  aorc.time                           =  111111112;


  // populate the evapotranspiration forcing data structure:
  // this part of code does not explicitly setting values, moved to et_wrapper_function()


  // ET forcing values that come from somewhere else...
  //---------------------------------------------------------------------------------------------------------------
  et_forcing.canopy_resistance_sec_per_m   = 50.0; // TODO: from plant growth model
  et_forcing.water_temperature_C           = 15.5; // TODO: from soil or lake thermal model
  et_forcing.ground_heat_flux_W_per_sq_m=-10.0;    // TODO from soil thermal model.  Negative denotes downward.

  if(et_options.yes_aorc==TRUE)
  {
    et_params.wind_speed_measurement_height_m=10.0;  // AORC uses 10m.  Must convert to wind speed at 2 m height.
  }    
  et_params.humidity_measurement_height_m=2.0; 
  et_params.vegetation_height_m=0.12;   // used for unit test of aerodynamic resistance used in Penman Monteith method.     
  et_params.zero_plane_displacement_height_m=0.0003;  // 0.03 cm for unit testing
  et_params.momentum_transfer_roughness_length_m=0.0;  // zero means that default values will be used in routine.
  et_params.heat_transfer_roughness_length_m=0.0;      // zero means that default values will be used in routine.


  // surface radiation parameter values that are a function of land cover.   Must be assigned from land cover type.
  //---------------------------------------------------------------------------------------------------------------
  surf_rad_params.surface_longwave_emissivity=1.0; // this is 1.0 for granular surfaces, maybe 0.97 for water
  surf_rad_params.surface_shortwave_albedo=0.22;  // this is a function of solar elev. angle for most surfaces.   


  if(et_options.yes_aorc!=TRUE)
  {
    // these values are needed if we don't have incoming longwave radiation measurements.
    surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
  }

    // these values are needed if we don't have incoming longwave radiation measurements.
    surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
    

  // Surface radiation forcing parameter values that must come from other models
  //---------------------------------------------------------------------------------------------------------------
  surf_rad_forcing.surface_skin_temperature_C = 12.0;  // TODO from soil thermal model or vegetation model.

  if(et_options.shortwave_radiation_provided=FALSE)
  {
    // populate the elements of the structures needed to calculate shortwave (solar) radiation, and calculate it
    // ### OPTIONS ###
    solar_options.cloud_base_height_known=FALSE;  // set to TRUE if the solar_forcing.cloud_base_height_m is known.

    // ### PARAMS ###
    solar_params.latitude_degrees      =  37.25;   // THESE VALUES ARE FOR THE UNIT TEST
    solar_params.longitude_degrees     = -97.5554; // THESE VALUES ARE FOR THE UNIT TEST
    solar_params.site_elevation_m      = 303.333;  // THESE VALUES ARE FOR THE UNIT TEST  

    // ### FORCING ###
    solar_forcing.cloud_cover_fraction         =   0.5;   // THESE VALUES ARE FOR THE UNIT TEST 
    solar_forcing.atmospheric_turbidity_factor =   2.0;   // 2.0 = clear mountain air, 5.0= smoggy air
    solar_forcing.day_of_year                  =  208;    // THESE VALUES ARE FOR THE UNIT TEST
    solar_forcing.zulu_time_h                  =  20.567; // THESE VALUES ARE FOR THE UNIT TEST

    // UNIT TEST RESULTS
    // CALCULATED SOLAR FLUXES
    // at time:     20.56700000 UTC
    // at site latitude: 37.250000 deg. longitude:-97.555400 deg.  elevation:303.333000 m
    // Shortwave radiation clear-sky flux calculations:
    // -above canopy/snow perpendicular to Earth-Sun line is      =964.56166277 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =661.40396086 W/m2
    // Shortwave radiation clear-sky flux calculations with 0.5000 cloud cover fraction:
    // -above canopy/snow perpendicular to Earth-Sun line is      =807.82039257 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =553.92581722 W/m2
    // CALCULATED ANGLES DESCRIBING VECTOR POINTING TO THE SUN
    // solar elevation angle:     43.29101185 degrees
    // solar azimuth:            225.06371958 degrees
    // local hour angle:          31.01549773 degrees
    // Number of tests passed=7 of 7.
    // UNIT TEST PASSED.
  }

  //et_method_option = 1;    use_energy_balance_method
  //et_method_option = 2;    use_aerodynamic_method
  //et_method_option = 3;    use_combination_method
  //et_method_option = 4;    use_priestley_taylor_method
  //et_method_option = 5;    use_penman_monteith_method
  et_method_option = 3;

  //std::cout << "Input et_method_option: 1-5" << std::endl;
  //std::cin >> et_method_option;

  //the following two variables are set in function et_function_calc_wrapper()
  //set_et_options->yes_aorc=TRUE;
  //set_et_options->shortwave_radiation_provided=FALSE;
  set_et_options.use_energy_balance_method   = FALSE;
  set_et_options.use_aerodynamic_method      = FALSE;
  set_et_options.use_combination_method      = FALSE;
  set_et_options.use_priestley_taylor_method = FALSE;
  set_et_options.use_penman_monteith_method  = FALSE;

  if (et_method_option == 1)
    set_et_options.use_energy_balance_method   = TRUE;
  if (et_method_option == 2)
    set_et_options.use_aerodynamic_method      = TRUE;
  if (et_method_option == 3)
    set_et_options.use_combination_method      = TRUE;
  if (et_method_option == 4)
    set_et_options.use_priestley_taylor_method = TRUE;
  if (et_method_option == 5)
    set_et_options.use_penman_monteith_method  = TRUE;

  et_m_per_s = et_wrapper_function(&aorc, &solar_options, &solar_params, &solar_forcing,
                                   &set_et_options, &et_params, &et_forcing, 
                                   &surf_rad_params, &surf_rad_forcing);

  printf("In main: calculated instantaneous potential evapotranspiration (PET) =%8.6e m/s\n",et_m_per_s);

  //EXPECT_DOUBLE_EQ (8.694909e-08, et_m_per_s);
  EXPECT_LT(abs(et_m_per_s-8.694909e-08), 1.0e-08);
  ASSERT_TRUE(true);

}


TEST_F(EtCalcKernelTest, TestPriestleyTaylorMethod)
{
  struct evapotranspiration_options set_et_options;
  double et_m_per_s;
  int et_method_option;


  // FLAGS
  int yes_aorc; // if TRUE then using AORC forcing data- if FALSE then we must calculate incoming short/longwave rad.
  int yes_wrf;  // if TRUE then we get radiation winds etc. from WRF output.  TODO not implemented.

  struct aorc_forcing_data aorc;

  struct evapotranspiration_options et_options;
  struct evapotranspiration_params  et_params;
  struct evapotranspiration_forcing et_forcing;
  struct intermediate_vars inter_vars;

  struct surface_radiation_params   surf_rad_params;
  struct surface_radiation_forcing  surf_rad_forcing;

  struct solar_radiation_options    solar_options; 
  struct solar_radiation_parameters solar_params;
  struct solar_radiation_forcing    solar_forcing;
  struct solar_radiation_results    solar_results;


  double wind_speed_at_2_m;
  double et_mm_per_d;
  double numerator,denominator;
  double saturation_vapor_pressure_Pa;
  double actual_vapor_pressure_Pa;

  //###################################################################################################
  // THE VALUE OF THESE FLAGS DETERMINE HOW THIS CODE BEHAVES.  CYCLE THROUGH THESE FOR THE UNIT TEST.
  //###################################################################################################
  // Set this flag to TRUE if meteorological inputs come from AORC
  set_et_options.yes_aorc = TRUE;                      // if TRUE, it means that we are using AORC data.
  set_et_options.shortwave_radiation_provided = FALSE;
  et_options.yes_aorc = set_et_options.yes_aorc;
  et_options.shortwave_radiation_provided = set_et_options.shortwave_radiation_provided;


  // -----UNIT TEST RESULTS:-----

  // -----Priestley-Taylor method: -----
  // calculated instantaneous potential evapotranspiration (PET) =8.249098e-08 m/s
  // calculated instantaneous potential evapotranspiration (PET) =7.127221 mm/d
  

  //###################################################################################################
  // MAKE UP SOME TYPICAL AORC DATA.  THESE VALUES DRIVE THE UNIT TESTS.
  //###################################################################################################
  //read_aorc_data().  TODO: These data come from some aorc reading/parsing function.
  //---------------------------------------------------------------------------------------------------------------

  aorc.incoming_longwave_W_per_m2     =  117.1;
  aorc.incoming_shortwave_W_per_m2    =  599.7;
  aorc.surface_pressure_Pa            =  101300.0;
  aorc.specific_humidity_2m_kg_per_kg =  0.00778;      // results in a relative humidity of 40%
  aorc.air_temperature_2m_K           =  25.0+TK;
  aorc.u_wind_speed_10m_m_per_s       =  1.54;
  aorc.v_wind_speed_10m_m_per_s       =  3.2;
  aorc.latitude                       =  37.865211;
  aorc.longitude                      =  -98.12345;
  aorc.time                           =  111111112;


  // populate the evapotranspiration forcing data structure:
  // this part of code does not explicitly setting values, moved to et_wrapper_function()


  // ET forcing values that come from somewhere else...
  //---------------------------------------------------------------------------------------------------------------
  et_forcing.canopy_resistance_sec_per_m   = 50.0; // TODO: from plant growth model
  et_forcing.water_temperature_C           = 15.5; // TODO: from soil or lake thermal model
  et_forcing.ground_heat_flux_W_per_sq_m=-10.0;    // TODO from soil thermal model.  Negative denotes downward.

  if(et_options.yes_aorc==TRUE)
  {
    et_params.wind_speed_measurement_height_m=10.0;  // AORC uses 10m.  Must convert to wind speed at 2 m height.
  }    
  et_params.humidity_measurement_height_m=2.0; 
  et_params.vegetation_height_m=0.12;   // used for unit test of aerodynamic resistance used in Penman Monteith method.     
  et_params.zero_plane_displacement_height_m=0.0003;  // 0.03 cm for unit testing
  et_params.momentum_transfer_roughness_length_m=0.0;  // zero means that default values will be used in routine.
  et_params.heat_transfer_roughness_length_m=0.0;      // zero means that default values will be used in routine.


  // surface radiation parameter values that are a function of land cover.   Must be assigned from land cover type.
  //---------------------------------------------------------------------------------------------------------------
  surf_rad_params.surface_longwave_emissivity=1.0; // this is 1.0 for granular surfaces, maybe 0.97 for water
  surf_rad_params.surface_shortwave_albedo=0.22;  // this is a function of solar elev. angle for most surfaces.   


  if(et_options.yes_aorc!=TRUE)
  {
    // these values are needed if we don't have incoming longwave radiation measurements.
    surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
  }

    // these values are needed if we don't have incoming longwave radiation measurements.
    surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
    

  // Surface radiation forcing parameter values that must come from other models
  //---------------------------------------------------------------------------------------------------------------
  surf_rad_forcing.surface_skin_temperature_C = 12.0;  // TODO from soil thermal model or vegetation model.

  if(et_options.shortwave_radiation_provided=FALSE)
  {
    // populate the elements of the structures needed to calculate shortwave (solar) radiation, and calculate it
    // ### OPTIONS ###
    solar_options.cloud_base_height_known=FALSE;  // set to TRUE if the solar_forcing.cloud_base_height_m is known.

    // ### PARAMS ###
    solar_params.latitude_degrees      =  37.25;   // THESE VALUES ARE FOR THE UNIT TEST
    solar_params.longitude_degrees     = -97.5554; // THESE VALUES ARE FOR THE UNIT TEST
    solar_params.site_elevation_m      = 303.333;  // THESE VALUES ARE FOR THE UNIT TEST  

    // ### FORCING ###
    solar_forcing.cloud_cover_fraction         =   0.5;   // THESE VALUES ARE FOR THE UNIT TEST 
    solar_forcing.atmospheric_turbidity_factor =   2.0;   // 2.0 = clear mountain air, 5.0= smoggy air
    solar_forcing.day_of_year                  =  208;    // THESE VALUES ARE FOR THE UNIT TEST
    solar_forcing.zulu_time_h                  =  20.567; // THESE VALUES ARE FOR THE UNIT TEST

    // UNIT TEST RESULTS
    // CALCULATED SOLAR FLUXES
    // at time:     20.56700000 UTC
    // at site latitude: 37.250000 deg. longitude:-97.555400 deg.  elevation:303.333000 m
    // Shortwave radiation clear-sky flux calculations:
    // -above canopy/snow perpendicular to Earth-Sun line is      =964.56166277 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =661.40396086 W/m2
    // Shortwave radiation clear-sky flux calculations with 0.5000 cloud cover fraction:
    // -above canopy/snow perpendicular to Earth-Sun line is      =807.82039257 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =553.92581722 W/m2
    // CALCULATED ANGLES DESCRIBING VECTOR POINTING TO THE SUN
    // solar elevation angle:     43.29101185 degrees
    // solar azimuth:            225.06371958 degrees
    // local hour angle:          31.01549773 degrees
    // Number of tests passed=7 of 7.
    // UNIT TEST PASSED.
  }

  //et_method_option = 1;    use_energy_balance_method
  //et_method_option = 2;    use_aerodynamic_method
  //et_method_option = 3;    use_combination_method
  //et_method_option = 4;    use_priestley_taylor_method
  //et_method_option = 5;    use_penman_monteith_method
  et_method_option = 4;

  //std::cout << "Input et_method_option: 1-5" << std::endl;
  //std::cin >> et_method_option;

  //the following two variables are set in function et_function_calc_wrapper()
  //set_et_options->yes_aorc=TRUE;
  //set_et_options->shortwave_radiation_provided=FALSE;
  set_et_options.use_energy_balance_method   = FALSE;
  set_et_options.use_aerodynamic_method      = FALSE;
  set_et_options.use_combination_method      = FALSE;
  set_et_options.use_priestley_taylor_method = FALSE;
  set_et_options.use_penman_monteith_method  = FALSE;

  if (et_method_option == 1)
    set_et_options.use_energy_balance_method   = TRUE;
  if (et_method_option == 2)
    set_et_options.use_aerodynamic_method      = TRUE;
  if (et_method_option == 3)
    set_et_options.use_combination_method      = TRUE;
  if (et_method_option == 4)
    set_et_options.use_priestley_taylor_method = TRUE;
  if (et_method_option == 5)
    set_et_options.use_penman_monteith_method  = TRUE;

  et_m_per_s = et_wrapper_function(&aorc, &solar_options, &solar_params, &solar_forcing,
                                   &set_et_options, &et_params, &et_forcing, 
                                   &surf_rad_params, &surf_rad_forcing);

  printf("In main: calculated instantaneous potential evapotranspiration (PET) =%8.6e m/s\n",et_m_per_s);

  //EXPECT_DOUBLE_EQ (8.249098e-08, et_m_per_s);
  EXPECT_LT(abs(et_m_per_s-8.249098e-08), 1.0e-08);
  ASSERT_TRUE(true);

}


TEST_F(EtCalcKernelTest, TestPenmanMonteithMethod)
{
  struct evapotranspiration_options set_et_options;
  double et_m_per_s;
  int et_method_option;


  // FLAGS
  int yes_aorc; // if TRUE then using AORC forcing data- if FALSE then we must calculate incoming short/longwave rad.
  int yes_wrf;  // if TRUE then we get radiation winds etc. from WRF output.  TODO not implemented.

  struct aorc_forcing_data aorc;

  struct evapotranspiration_options et_options;
  struct evapotranspiration_params  et_params;
  struct evapotranspiration_forcing et_forcing;
  struct intermediate_vars inter_vars;

  struct surface_radiation_params   surf_rad_params;
  struct surface_radiation_forcing  surf_rad_forcing;

  struct solar_radiation_options    solar_options; 
  struct solar_radiation_parameters solar_params;
  struct solar_radiation_forcing    solar_forcing;
  struct solar_radiation_results    solar_results;


  double wind_speed_at_2_m;
  double et_mm_per_d;
  double numerator,denominator;
  double saturation_vapor_pressure_Pa;
  double actual_vapor_pressure_Pa;

  //###################################################################################################
  // THE VALUE OF THESE FLAGS DETERMINE HOW THIS CODE BEHAVES.  CYCLE THROUGH THESE FOR THE UNIT TEST.
  //###################################################################################################
  // Set this flag to TRUE if meteorological inputs come from AORC
  set_et_options.yes_aorc = TRUE;                      // if TRUE, it means that we are using AORC data.
  set_et_options.shortwave_radiation_provided = FALSE;
  et_options.yes_aorc = set_et_options.yes_aorc;
  et_options.shortwave_radiation_provided = set_et_options.shortwave_radiation_provided;


  // -----UNIT TEST RESULTS:-----

  //(NOTE: aerodynamic roughness terms are different in this test, calculated from veg. height as per FAO method)
  // Penman Monteith method:
  // calculated instantaneous potential evapotranspiration (PET) =1.106268e-07 m/s
  // calculated instantaneous potential evapotranspiration (PET) =9.558159 mm/d
  

  //###################################################################################################
  // MAKE UP SOME TYPICAL AORC DATA.  THESE VALUES DRIVE THE UNIT TESTS.
  //###################################################################################################
  //read_aorc_data().  TODO: These data come from some aorc reading/parsing function.
  //---------------------------------------------------------------------------------------------------------------

  aorc.incoming_longwave_W_per_m2     =  117.1;
  aorc.incoming_shortwave_W_per_m2    =  599.7;
  aorc.surface_pressure_Pa            =  101300.0;
  aorc.specific_humidity_2m_kg_per_kg =  0.00778;      // results in a relative humidity of 40%
  aorc.air_temperature_2m_K           =  25.0+TK;
  aorc.u_wind_speed_10m_m_per_s       =  1.54;
  aorc.v_wind_speed_10m_m_per_s       =  3.2;
  aorc.latitude                       =  37.865211;
  aorc.longitude                      =  -98.12345;
  aorc.time                           =  111111112;


  // populate the evapotranspiration forcing data structure:
  // this part of code does not explicitly setting values, moved to et_wrapper_function()


  // ET forcing values that come from somewhere else...
  //---------------------------------------------------------------------------------------------------------------
  et_forcing.canopy_resistance_sec_per_m   = 50.0; // TODO: from plant growth model
  et_forcing.water_temperature_C           = 15.5; // TODO: from soil or lake thermal model
  et_forcing.ground_heat_flux_W_per_sq_m=-10.0;    // TODO from soil thermal model.  Negative denotes downward.

  if(et_options.yes_aorc==TRUE)
  {
    et_params.wind_speed_measurement_height_m=10.0;  // AORC uses 10m.  Must convert to wind speed at 2 m height.
  }    
  et_params.humidity_measurement_height_m=2.0; 
  et_params.vegetation_height_m=0.12;   // used for unit test of aerodynamic resistance used in Penman Monteith method.     
  et_params.zero_plane_displacement_height_m=0.0003;  // 0.03 cm for unit testing
  et_params.momentum_transfer_roughness_length_m=0.0;  // zero means that default values will be used in routine.
  et_params.heat_transfer_roughness_length_m=0.0;      // zero means that default values will be used in routine.


  // surface radiation parameter values that are a function of land cover.   Must be assigned from land cover type.
  //---------------------------------------------------------------------------------------------------------------
  surf_rad_params.surface_longwave_emissivity=1.0; // this is 1.0 for granular surfaces, maybe 0.97 for water
  surf_rad_params.surface_shortwave_albedo=0.22;  // this is a function of solar elev. angle for most surfaces.   


  if(et_options.yes_aorc!=TRUE)
  {
    // these values are needed if we don't have incoming longwave radiation measurements.
    surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
  }

    // these values are needed if we don't have incoming longwave radiation measurements.
    surf_rad_forcing.incoming_shortwave_radiation_W_per_sq_m     = 440.1;     // must come from somewhere
    surf_rad_forcing.incoming_longwave_radiation_W_per_sq_m      = -1.0e+05;  // this huge negative value tells to calc.
    surf_rad_forcing.air_temperature_C                           = 15.0;      // from some forcing data file
    surf_rad_forcing.relative_humidity_percent                   = 63.0;      // from some forcing data file
    surf_rad_forcing.ambient_temperature_lapse_rate_deg_C_per_km = 6.49;      // ICAO standard atmosphere lapse rate
    surf_rad_forcing.cloud_cover_fraction                        = 0.6;       // from some forcing data file
    surf_rad_forcing.cloud_base_height_m                         = 2500.0/3.281; // assumed 2500 ft.
    

  // Surface radiation forcing parameter values that must come from other models
  //---------------------------------------------------------------------------------------------------------------
  surf_rad_forcing.surface_skin_temperature_C = 12.0;  // TODO from soil thermal model or vegetation model.

  if(et_options.shortwave_radiation_provided=FALSE)
  {
    // populate the elements of the structures needed to calculate shortwave (solar) radiation, and calculate it
    // ### OPTIONS ###
    solar_options.cloud_base_height_known=FALSE;  // set to TRUE if the solar_forcing.cloud_base_height_m is known.

    // ### PARAMS ###
    solar_params.latitude_degrees      =  37.25;   // THESE VALUES ARE FOR THE UNIT TEST
    solar_params.longitude_degrees     = -97.5554; // THESE VALUES ARE FOR THE UNIT TEST
    solar_params.site_elevation_m      = 303.333;  // THESE VALUES ARE FOR THE UNIT TEST  

    // ### FORCING ###
    solar_forcing.cloud_cover_fraction         =   0.5;   // THESE VALUES ARE FOR THE UNIT TEST 
    solar_forcing.atmospheric_turbidity_factor =   2.0;   // 2.0 = clear mountain air, 5.0= smoggy air
    solar_forcing.day_of_year                  =  208;    // THESE VALUES ARE FOR THE UNIT TEST
    solar_forcing.zulu_time_h                  =  20.567; // THESE VALUES ARE FOR THE UNIT TEST

    // UNIT TEST RESULTS
    // CALCULATED SOLAR FLUXES
    // at time:     20.56700000 UTC
    // at site latitude: 37.250000 deg. longitude:-97.555400 deg.  elevation:303.333000 m
    // Shortwave radiation clear-sky flux calculations:
    // -above canopy/snow perpendicular to Earth-Sun line is      =964.56166277 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =661.40396086 W/m2
    // Shortwave radiation clear-sky flux calculations with 0.5000 cloud cover fraction:
    // -above canopy/snow perpendicular to Earth-Sun line is      =807.82039257 W/m2
    // -at the top of a horizontal canopy/snow surface is:        =553.92581722 W/m2
    // CALCULATED ANGLES DESCRIBING VECTOR POINTING TO THE SUN
    // solar elevation angle:     43.29101185 degrees
    // solar azimuth:            225.06371958 degrees
    // local hour angle:          31.01549773 degrees
    // Number of tests passed=7 of 7.
    // UNIT TEST PASSED.
  }

  //et_method_option = 1;    use_energy_balance_method
  //et_method_option = 2;    use_aerodynamic_method
  //et_method_option = 3;    use_combination_method
  //et_method_option = 4;    use_priestley_taylor_method
  //et_method_option = 5;    use_penman_monteith_method
  et_method_option = 5;

  //std::cout << "Input et_method_option: 1-5" << std::endl;
  //std::cin >> et_method_option;

  //the following two variables are set in function et_function_calc_wrapper()
  //set_et_options->yes_aorc=TRUE;
  //set_et_options->shortwave_radiation_provided=FALSE;
  set_et_options.use_energy_balance_method   = FALSE;
  set_et_options.use_aerodynamic_method      = FALSE;
  set_et_options.use_combination_method      = FALSE;
  set_et_options.use_priestley_taylor_method = FALSE;
  set_et_options.use_penman_monteith_method  = FALSE;

  if (et_method_option == 1)
    set_et_options.use_energy_balance_method   = TRUE;
  if (et_method_option == 2)
    set_et_options.use_aerodynamic_method      = TRUE;
  if (et_method_option == 3)
    set_et_options.use_combination_method      = TRUE;
  if (et_method_option == 4)
    set_et_options.use_priestley_taylor_method = TRUE;
  if (et_method_option == 5)
    set_et_options.use_penman_monteith_method  = TRUE;

  et_m_per_s = et_wrapper_function(&aorc, &solar_options, &solar_params, &solar_forcing,
                                   &set_et_options, &et_params, &et_forcing, 
                                   &surf_rad_params, &surf_rad_forcing);

  printf("In main: calculated instantaneous potential evapotranspiration (PET) =%8.6e m/s\n",et_m_per_s);

  //EXPECT_DOUBLE_EQ (1.106268e-07, et_m_per_s);
  EXPECT_LT(abs(et_m_per_s-1.106268e-07), 1.0e-07);
  ASSERT_TRUE(true);

}
