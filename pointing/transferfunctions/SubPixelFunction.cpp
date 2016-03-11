/* -*- mode: c++ -*-
 *
 * pointing/transferfunctions/SubPixelFunction.cpp --
 *
 * Initial software
 * Authors: Izzat Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

#include <pointing/transferfunctions/SubPixelFunction.h>
#include <math.h>

#define         DEFAULT_RESOLUTION_HUMAN       400
#define         MIN(X, Y)              (((X) < (Y)) ? (X) : (Y))

namespace pointing
{
  URI SubPixelFunction::decodeURI(URI &uri)
  {
    std::string encodedUri;
    URI::getQueryArg(uri.query, "transFunc", &encodedUri);
    return URI::decode(encodedUri);
  }

  SubPixelFunction::SubPixelFunction(const char *uriString, PointingDevice *input, DisplayDevice *output)
  {
    URI uri(uriString);
    URI decodedURI(uri);
    initialize(uri, decodedURI, input, output);
  }

  SubPixelFunction::SubPixelFunction(std::string uriString, PointingDevice *input, DisplayDevice *output)
  {
    URI uri(uriString);
    URI decodedURI(uri);
    initialize(uri, decodedURI, input, output);
  }

  SubPixelFunction::SubPixelFunction(URI &uri, PointingDevice *input, DisplayDevice *output)
  {
    std::string encodedUri;
    URI::getQueryArg(uri.query, "transFunc", &encodedUri);

    URI funcUri(URI::decode(encodedUri));
    initialize(uri, funcUri, input, output);
  }

  void SubPixelFunction::initialize(URI &uri, URI &funcUri, PointingDevice *input, DisplayDevice *output)
  {
    debugLevel = 0 ;
    URI::getQueryArg(uri.query, "debugLevel", &debugLevel) ;

    func = TransferFunction::create(funcUri, input, output);

    isOn = true;
    cardinality = widgetSize = lastTime = 0;
    URI::getQueryArg(uri.query, "isOn", &isOn);
    URI::getQueryArg(uri.query, "cardinality", &cardinality);
    URI::getQueryArg(uri.query, "widgetsize", &widgetSize);

    int resHuman = DEFAULT_RESOLUTION_HUMAN;
    URI::getQueryArg(uri.query, "resHuman", &resHuman);

    this->input = input;
    this->output = output;

    setHumanResolution(resHuman);
    minGainAndVelocity();
    computeParameters();
  }

  SubPixelFunction::SubPixelFunction(URI uri, URI funcUri, PointingDevice *input, DisplayDevice *output)
  {
    initialize(uri, funcUri, input, output);
  }

  void SubPixelFunction::minGainAndVelocity()
  {
    int x = 0;
    double resx = 0, resy;
    int sleepTimeInMs = int(1000 / input->getUpdateFrequency());
    while (resx < 1 && ++x < 128)
    {
      func->applyd(x, 0, &resx, &resy, TimeStamp::createAsInt());
      PointingDevice::idle(sleepTimeInMs);
    }
    Gpix = resx / x;
    if (debugLevel)
      std::cerr << "SubPixelFunction::minGainAndVelocity: Gpix=" << Gpix << std::endl;
  }

  void SubPixelFunction::computeParameters()
  {
    if (cardinality && widgetSize)
    {
      Gopt = widgetSize * resUseful / cardinality / output->getResolution();
      Vuse = 25.4 * input->getUpdateFrequency() / resUseful;
      Vpix = 25.4 * input->getUpdateFrequency() / output->getResolution() / Gpix ;
      if (debugLevel)
      {
        std::cerr << "gOpt: " << Gopt << std::endl;
        std::cerr << "vUse: " << Vuse << std::endl;
        std::cerr << "Vpix: " << Vpix << std::endl;
      }
    }
  }

  void SubPixelFunction::setSubPixeling(bool subpixeling)
  {
    this->isOn = subpixeling;
  }

  bool SubPixelFunction::getSubPixeling()
  {
    return this->isOn;
  }

  void SubPixelFunction::setHumanResolution(int resHuman)
  {
    this->resUseful = MIN(DEFAULT_RESOLUTION_HUMAN, input->getResolution());
    if (resHuman > 200 && resHuman < this->resUseful)
      this->resUseful = (float)resHuman;
  }

  int SubPixelFunction::getHumanResolution()
  {
    return this->resUseful;
  }

  void SubPixelFunction::setCardinalitySize(int cardinality, int size)
  {
    this->cardinality = cardinality;
    this->widgetSize = size;
    computeParameters();
  }

  void SubPixelFunction::getCardinalitySize(int *cardinality, int *size)
  {
    *cardinality = this->cardinality;
    *size = this->widgetSize;
  }

  void SubPixelFunction::clearState()
  {
    func->clearState();
  }

  void SubPixelFunction::applyi(int dxMickey, int dyMickey, int *dxPixel, int *dyPixel, TimeStamp::inttime timestamp)
  {
    func->applyi(dxMickey, dyMickey, dxPixel, dyPixel, timestamp);
  }

  void SubPixelFunction::applyd(int dxMickey, int dyMickey, double *dxPixel, double *dyPixel, TimeStamp::inttime timestamp)
  {
    if (isOn)
    {
      TimeStamp::inttime now = TimeStamp::createAsInt();
      double dtInSec = (now - lastTime) / 1000000000.;
      //std::cout << "dtInSec: " << dtInSec << std::endl;
      lastTime = now;

      double dxInMm = dxMickey / input->getResolution() * 25.4;
      double dyInMm = dyMickey / input->getResolution() * 25.4;
      double ddInMm = sqrt(dxInMm * dxInMm + dyInMm * dyInMm);
      double speedInMmPerSec = ddInMm / dtInSec;
      double outDx = 0, outDy = 0;
      func->applyd(dxMickey, dyMickey, &outDx, &outDy, timestamp);
      double gain = sqrt(outDx*outDx + outDy*outDy) / ddInMm;

      if (debugLevel > 1) std::cerr << "Original gain: " << gain << std::endl;
      if (Vpix > Vuse)
      {
        double q = (speedInMmPerSec - Vuse) / (Vpix - Vuse);
        if(speedInMmPerSec <= Vuse) {
          gain = Gopt;
        } else if(speedInMmPerSec <= Vpix) {
          gain = (1-q) * Gopt + q * gain;
        }
      }
      else
      {
        double q = speedInMmPerSec / Vpix;
        if(q < 1) {
          gain = (1-q) * Gopt + q * gain;
        }
      }
      *dxPixel = gain * dxInMm;
      *dyPixel = gain * dyInMm;

      if (debugLevel > 1) std::cerr << "Computed gain: " << gain << std::endl;

      lastTime = TimeStamp::createAsInt();
    }
    else func->applyd(dxMickey, dyMickey, dxPixel, dyPixel, timestamp);
  }

  URI SubPixelFunction::getURI(bool expanded) const
  {
    std::string encodedTF = URI::encode(func->getURI(expanded).asString());
    URI uri;
    uri.scheme = "subpixel";
    URI::addQueryArg(uri.query, "cardinality", cardinality);
    URI::addQueryArg(uri.query, "widgetSize", widgetSize);
    URI::addQueryArg(uri.query, "transFunc", encodedTF);
    if (expanded || resUseful != DEFAULT_RESOLUTION_HUMAN)
      URI::addQueryArg(uri.query, "resHuman", resUseful);
    return uri ;
  }

  URI SubPixelFunction::getInnerURI(bool expanded) const
  {
    return func->getURI(expanded);
  }


  SubPixelFunction::~SubPixelFunction()
  {
    delete func;
  }
}
