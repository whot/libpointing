/* -*- mode: c++ -*-
 *
 * apps/consoleExample/consoleExample.cpp --
 *
 * Initial software
 * Authors: Gery Casiez, Nicolas Roussel
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include <pointing/pointing.h>

#include <iomanip>
#include <stdexcept>

using namespace pointing ;

TransferFunction *func = 0 ;
TimeStamp::inttime last_time = 0 ;
bool button_pressed = false ;

void
pointingCallback(void * /*context*/, TimeStamp::inttime timestamp,
		 int input_dx, int input_dy, int buttons) {
  if (!func) return ;

  int output_dx=0, output_dy=0 ;
  func->applyi(input_dx, input_dy, &output_dx, &output_dy, timestamp) ;
  double delta = (double)(timestamp - last_time)/TimeStamp::one_millisecond ;
  if (delta<=0.0) std::cout << std::endl ;
  std::cout
    << timestamp << " ns (" << TimeStamp::createAsStringFrom(timestamp) << "), " 
    << std::setw(7) << delta << " ms later (" << std::setw(7) << (1000.0/delta) << " Hz), "
    << "(" << std::setw(3) << input_dx << ", " << std::setw(3) << input_dy << ") counts"
    << " -> (" << std::setw(3) << output_dx << ", " << std::setw(3) << output_dy << ") pixels, "
    << "buttons: " << buttons << std::endl ;
  if (delta<=0.0) std::cout << std::endl ;
  last_time = timestamp;
  button_pressed = buttons&PointingDevice::BUTTON_2 ;
}

int
main(int argc, char** argv) {
  try {

    if (argc < 1)
      std::cerr << "Usage: " << argv[0]
		<< " [inputdeviceURI [outputdeviceURI [transferfunctionURI]]]"
		<< std::endl  ;

    // --- Pointing device ----------------------------------------------------

    PointingDevice *input = PointingDevice::create(argc>1?argv[1]:"default:") ;
    for (TimeStamp reftime, now;
	 !input->isActive() && now-reftime<15*TimeStamp::one_second; 
	 now.refresh())
      PointingDevice::idle(500) ;

    std::cout << std::endl << "Pointing device" << std::endl ;
    std::cout << "  " << input->getURI(true).asString() << std::endl
	      << "  " << input->getResolution() << " CPI, " 
	      << input->getUpdateFrequency() << " Hz" << std::endl 
	      << "  device is " << (input->isActive()?"":"not ") << "active" << std::endl 
	      << std::endl ;

    // --- Display device -----------------------------------------------------

    DisplayDevice *output = DisplayDevice::create(argc>2?argv[2]:"default:") ;

    double hdpi, vdpi;
    output->getResolution(&hdpi, &vdpi) ;
    DisplayDevice::Size size = output->getSize() ;
    DisplayDevice::Bounds bounds = output->getBounds() ;
    std::cout << std::endl << "Display device" << std::endl
	      << "  " << output->getURI(true).asString() << std::endl
	      << "  " << bounds.size.width << " x " << bounds.size.height << " pixels, "
	      << size.width << " x " << size.height << " mm" << std::endl
	      << "  " << hdpi << " x " << vdpi << " PPI, "
	      << output->getRefreshRate() << " Hz" << std::endl ;

    // --- Transfer function --------------------------------------------------

    func = TransferFunction::create(argc>3?argv[3]:"system:?debugLevel=2", input, output) ;

    std::cout << std::endl << "Transfer function" << std::endl
	      << "  " << func->getURI(true).asString() << std::endl
	      << std::endl ;

    // --- Ready to go --------------------------------------------------------

    input->setPointingCallback(pointingCallback) ;
    while (!button_pressed || 1)
      PointingDevice::idle(100) ; // milliseconds

    // --- Done ---------------------------------------------------------------

    delete input ;
    delete output ;
    delete func ;

  } catch (std::runtime_error e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception e) {
    std::cerr << "Exception: " << e.what() << std::endl ;
  }

  return 0 ;
}
