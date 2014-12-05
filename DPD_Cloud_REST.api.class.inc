<?php
/**
 * DPD REST API library/class
 * Interaction with DPD REST Web services.
 */
class DPD_Cloud_REST {

   private $Url; // URL for RETS service.
   private $Version; // Version.
   private $Language; // Message language.
   private $PartnerName; // Name of your API partner.
   private $PartnerToken; // Token of your API partner.
   private $UsercloudUserID; // CustomerID of the DPD customer. Assigned by DPD.
   private $UserToken; // Customer password of the DPD customer. Assigned by DPD.

  /**
   * Constructor which initializes the consumer.
   *
   * @param string $partner_name
   *   Name of your API partner.
   * @param string $partner_token
   *   Token of your API partner.
   * @param string $user_clouduserid
   *   CustomerID of the DPD customer. Assigned by DPD.
   * @param string $user_token
   *   TCustomer password of the DPD customer. Assigned by DPD.
   */
  public function __construct($partner_name, $partner_token, $user_clouduserid, $user_token, $language = 'en_EN') {
    $this->Url = 'https://cloud-stage.dpd.com/api/v1/';
    $this->PartnerName = $partner_name;
    $this->PartnerToken = $partner_token;
    $this->UsercloudUserID = $user_clouduserid;
    $this->UserToken = $user_token;
    $this->Version = '100';
    $this->Language = $language;
  }

  /**
   * Make a REST request to a DPD service.
   *
   * @param $data
   * @return array;
   */
  public function MakeRequest($method_url, $data = array(), $method = 'GET') {
    $cType = 'application/json';
    $defaultRequestHeaders = array(
      'Version : ' . $this->Version,
      'Language : ' . $this->Language,
      'PartnerCredentials-Name : ' . $this->PartnerName,
      'PartnerCredentials-Token : ' . $this->PartnerToken,
      'UserCredentials-cloudUserID : ' . $this->UsercloudUserID,
      'UserCredentials-Token : ' . $this->UserToken,
    );

    $curl = curl_init();
    $url = $this->Url . $method_url;
    if ($method == 'GET' && !empty($data)) {
      foreach ($data as $url_item) {
        // Encode the url items.
        $url .= '/' . rawurlencode($url_item);
      }
    }

    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($curl, CURLOPT_SSL_VERIFYHOST, false);
    if ($method == 'POST') {
      $data = json_encode($data);
      curl_setopt($curl, CURLOPT_POST, 1);
      curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
      // Add JSON content type to headers to be sent.
      // Important DPD API wants this header to be first in the headers array.
      array_unshift($defaultRequestHeaders, 'Content-Type: application/json');
    }
    else {
      curl_setopt($curl, CURLOPT_HTTPGET, TRUE);
    }
    curl_setopt($curl, CURLOPT_HTTPHEADER, $defaultRequestHeaders);
    $curl_response = curl_exec($curl);
    $code = curl_getinfo($curl, CURLINFO_HTTP_CODE);
    if ($curl_response === FALSE) {
        $info = curl_getinfo($curl);
        curl_close($curl);
        throw new Exception('Error occured during curl exec. Additional info: ' . var_export($info));
    }
    curl_close($curl);
    $response = json_decode($curl_response);
    if (isset($response->response->status) && $response->response->status == 'ERROR') {
        throw new Exception('Error occured: ' . $response->response->errormessage);
    }
    return $response;
  }

  /**
   * setOrder method REST request to a DPD service for multiple orders data.
   * Generates a DPD parcel label as Base64-String and starts a shipping order.
   *
   * @param $action - Enumeration of different order start types.
   * Possible:
   * . startOrder = 0 (Order start)
   * . checkOrderData = 1 (Order check only)
   * @param $settings - Settings for order start
   * @param $orders_data - Array contains orders data with their shipping address information,
   * parcel information, ParcelShop ID.
   * example:
    $order_data = array(
      'OrderAction' => $action,
        'OrderSettings' => array(
          'ShipDate' => date('m.d.Y'),
          'LabelSize' => 'PDF_A4',
          'LabelStartPosition' => 'UpperLeft',
    ),
     'OrderDataList' => array(
        // First order
        array(
         'ShipAddress' => array(
           'Company' => '',
           'Salutation' => '',
           'Name' => '',
           'Street' => '',
           'HouseNo' => '',
           'Country' => '',
           'ZipCode' => '',
           'City' => '',
           'State' => '',
           'Phone' => '',
           'Mail' => '',
          ),
         'ParcelShopID' => 0,
         'ParcelData' => array(
            'ShipService' => '',
            'Weight' => '',
            'Content' => '',
            'YourInternalID' => '',
            'Reference1' => '',
            'Reference2' => '',
            'COD' => array(
              'Purpose' => '',
              'Amount' => '',
              'Payment' => '',
            ),
          ),
        ),
        // Second order - same structure as for the first one
        // ...
     ),
    );
   * @return array;
   */
  public function setOrders($action, $settings = array(), $orders_data) {
    $settings += $this->defaultOrderSettings();
    foreach ($orders_data as $order_data) {
      $set_orders_data[] = array(
        'ShipAddress' => $order_data['ship_address'] + $this->defaultShipAddress(),
        'ParcelShopID' => !empty($order_data['parcel_shop_id']) ? $order_data['parcel_shop_id'] : 0,
        'ParcelData' => $order_data['parcel_data'] + $this->defaultParcelData(),
      );
    }
    $setOrderData = array(
      'OrderAction' => $action,
      'OrderSettings' => $settings,
      'OrderDataList' => $set_orders_data,
    );

    $response = $this->MakeRequest('setOrder', $setOrderData, 'POST');
    // $response->LabelResponse->LabelPDF = base64_decode($response->LabelResponse->LabelPDF);

    return $response;
  }

  /**
   * setOrder method REST request to a DPD service for single order data.
   * Generates a DPD parcel label as Base64-String and starts a shipping order.
   *
   * @param $action - Enumeration of different order start types.
   * @param $settings - Settings for order start
   * @param $ship_address - Array contains shipping address information.
   * @param $parcel_data - Array contains parcel information.
   * @param $parcel_shop_id - ParcelShop ID of the receiving ParcelShop. Relevant for shipping product "Shop_Delivery".
   * @return array;
   */
  public function setOrder($action, $settings = array(), $ship_address = array(), $parcel_data = array(), $parcel_shop_id = 0) {
    $settings += $this->defaultOrderSettings();
    $ship_address += $this->defaultShipAddress();
    $parcel_data += $this->defaultParcelData();
    $order_data = array(
      'OrderAction' => $action,
      'OrderSettings' => $settings,
      'OrderDataList' => array(
        array(
          'ShipAddress' => $ship_address,
          'ParcelShopID' => $parcel_shop_id,
          'ParcelData' => $parcel_data,
        ),
      ),
    );

    $response = $this->MakeRequest('setOrder', $order_data, 'POST');

    return $response;
  }

  /**
   * Constructs default Order Settings array.
   *
   * @param $additional_params
   * @return array;
   */
  private function defaultOrderSettings() {
    $orderSettings = array(
      'ShipDate' => date('m.d.Y'), // Shipping date (Format: dd.mm.yyyy)
      // Note: No parcel pick-up on Sunday and public holidays.
      // You get a list of valid ship days for a zip code using "getZipCodeRules".
      'LabelSize' => 'PDF_A4', // Enumeration of different label size types.
      // Possible values:
      // . PDF_A4 = 0
      // . PDF_A6 = 1
      // . ZPL_A6 = 2
      'LabelStartPosition' => 'UpperLeft', //Enumeration of the different label printing positions.
      // Possible values:
      // . UpperLeft = 0
      // . UpperRight = 1
      // . LowerLeft = 2
      // . LowerRight = 3
    );

    return $orderSettings;
  }

  /**
   * Constructs default Order Settings array.
   *
   * @return array;
   */
  private function defaultShipAddress() {
    $ShipAddress = array(
      'Company' => '', // Company name
      'Salutation' => '', // Salutation (e.g. Mr., Mrs.)
      'Name' => '', // Name of the contact person
      'Street' => '', // Street name
      'HouseNo' => '', // House number
      'Country' => '', // Possible Values: Alpha3, Alpha2, ISO3166, Country name
      // Example: DEU, DE, 276, Deutschland
      'ZipCode' => '', // Zip code of a city
      'City' => '', // City name, if applicable city district
      // Example:
      // Aschaffenburg, Obernau
      'State' => '', // ISO3166-2 code of a state
      // Important:
      // If „USA“ or „CAN“ is selected as country, state is mandatory!
      // For all other countries, it is not allowed to specify a state!
      'Phone' => '', // Phone number (mobile, too.)
      // Notes concerning allowed characters:
      // . numbers 0-9
      // . any number of "blanks"
      // . "+" and "-"
      // . "(" and ")"
      'Mail' => '', // E-mail address (all known standard formats, valid according to general understatement.)

    );

    return $ShipAddress;
  }

  /**
   * Constructs default Parcel Data array.
   *
   * @return array;
   */
  private function defaultParcelData() {
    $ParcelData = array(
      'ShipService' => '', // Enumeration of the DPD shipping products.
      // Possible values:
      // . Classic = 0
      // . Classic_Predict = 1
      // . Classic_COD = 2
      // . Classic_COD_Predict = 3
      // . Shop_Delivery = 4
      // . Shop_Return = 5
      // . Classic_Return = 6 (Only single order start!)
      'Weight' => '', // Parcel weight: 31,5 kg at maximum
      'Content' => '', // Parcel content description
      'YourInternalID' => '', // Internal reference field for linking the DPD parcel number with your internal system
      // (individual specification).
      'Reference1' => '', // Reference text 1 (individual specification)
      'Reference2' => '', // Reference text 2 (individual specification)
      // Example: DEU, DE, 276, Deutschland
      'COD' => array( // Contains "cash on delivery" information
        'Purpose' => '', // "Cash on delivery" Payment
        'Amount' => '', // "Cash on delivery" amount
        'Payment' => '', // Enumeration of payment type "COD"
        // Possible Values:
        // . Cash = 0 (Payment in cash)
        // . Cheque = 1 (Payment with cheque)
      ),
    );

    return $ParcelData;
  }

  /**
   * getParcelLifeCycle method REST request to a DPD service for a given parcel.
   * Returns the whole tracking data of a DPD Parcel by specifying the parcel number.
   *
   * @param $parcel_no - 14-digit parcel number (with preceding "0").
   * @return array;
   */
  public function getParcelLifeCycle($parcel_no) {
    $parcel_data = array(
      'ParcelNo' => $parcel_no,
    );

    $response = $this->MakeRequest('ParcelLifeCycle', $parcel_data);

    return $response;
  }

  /**
   * getParcelShopFinder method REST request to a DPD service.
   * Returns the master data of one ore more DPD ParcelShops
   * (100 simultaneously at maximum), as well as the unique ParcelShop identification number ("ParcelShopID").
   *
   * @param $search_data - an array with Address data or geo coordinatses.
   * @param $search_mode - the desired search mode type.
   * Possible values:
   * . SearchByAddress = 0 (Search via Shop address data)
   * . SearchByGeoData = 1 (Search via geo-coordinates).
   * @param $hide_on_closed_at - Fades out the ParcelShops, which are closed at a given date and time.
   * Format: yyyy-MM-dd HH:mm.
   * @param $search_geodata - an array with geo coordinates.
   * @param $need_service - Enumeration of the desired services, a ParcelShop offers.
   * Possible Values:
   * . Standard = 0
   * . ConsigneePickup = 1 (Pick-up by recipient)
   * . ReturnService = 2 (Return acceptance)
   * . ExpressService = 3 (Express shipping)
   * . PrepaidService = 4 (Parcel payment in advance)
   * . CashOnDeliveryService = 5 (Cash payment at delivery)
   * @param $max_results - Maximum amount of matches (100 simultaneously at maximum).
   * @return object response;
   */
  public function getParcelShopFinder($search_data, $search_mode = 0, $max_results = 100, $need_service = 0, $hide_on_closed_at = 'null') {
    if ($search_mode === 0) {
      $reponse = $this->getParcelShopFinderByAddress($search_data, $hide_on_closed_at, $need_service, $max_results = 100);
    }
    elseif($search_mode == 1) {
      $reponse = $this->getParcelShopFinderByGeoData($search_data['longitude'], $search_data['latitude'], $hide_on_closed_at, $need_service, $max_results = 100);
    }

    return $response;
  }

  /**
   * getParcelShopFinder by Address data.
   *
   * @param $search_address - an array with Address data or geo coordinates.
   * @return object response;
   */
  public function getParcelShopFinderByAddress($search_address, $max_results = 100, $need_service = 0, $hide_on_closed_at = 'null') {
    $default_search_address = $this->defaultSearchAddress();
    $search_address += $default_search_address;
    // Make sure we have the right order for array elements.
    $search_address = array_replace($default_search_address, array_intersect_key($search_address, $default_search_address));

    $search_params = array(
      'MaxReturnValues' => $max_results,
    )
    + $search_address +
    array(
      'NeedService' => $need_service,
      'HideOnClosedAt' => $hide_on_closed_at,
    );

    $response = $this->MakeRequest('ParcelShopFinder', $search_params);

    return $response;
  }

  /**
   * getParcelShopFinder by geo coordinates.
   *
   * @param $search_geodata - an array with geo coordinates.
   * @return object response;
   */
  public function getParcelShopFinderByGeoData($longitute, $latitude, $max_results = 100, $need_service = 0, $hide_on_closed_at = 'null') {
    $search_params = array(
      'MaxReturnValues' => $max_results,
      'Longitude' => $longitute,
      'Latitude' => $latitude,
      'NeedService' => $need_service,
      'HideOnClosedAt' => $hide_on_closed_at,
    );

    $response = $this->MakeRequest('ParcelShopFinder', $search_params);

    return $response;
  }

  /**
   * Constructs default Search address data array.
   *
   * @return array;
   */
  public function defaultSearchAddress() {
    $SearchAddress = array(
      'Street' => '', // Street name
      'HouseNo' => '', // House number
      'ZipCode' => '', // Zip code of a city
      'City' => '', // City name
      'Country' => '', // Possible Values: Alpha3, Alpha2, ISO3166, Country name
      // Example: DEU, DE, 276, Deutschland
    );

    return $SearchAddress;
  }

  /**
   * getZipCodeRules method REST request to a DPD service.
   * Returns general shipping information for a pick-up point:
   * - Weekdays, on which no parcels can be picked up (public holidays, location-dependent conditions)
   *   Note: Saturday and Sunday are not pick-up days in general.
   *   So these days are not part of the list.
   * - Responsible pick-up depot
   * - Latest pick-up times for a parcel pick-up today
   *
   * The method does not require any specification of certain parameters.
   * @return object response;
   */
  public function getZipCodeRules() {
    $response = $this->MakeRequest('ZipCodeRules');

    return $response;
  }


}
