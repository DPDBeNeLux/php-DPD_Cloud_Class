<?php

include_once 'DPD_Cloud_REST.api.class.inc';
$partner_name = 'Partner Name';
$partner_token = 'Partner Token';
$user_clouduserid = 'User Name';
$user_token = 'User password';
$dpd = new DPD_Cloud_REST($partner_name, $partner_token, $user_clouduserid, $user_token);

// Make an order request
// Fill with your values/data
$settings = array(
  'ShipDate' => '',
  'LabelSize' => '',
  'LabelStartPosition' => '',
);
$ship_address = array(
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
);
$parcel_shop_id = 0;
$parcel_data = array(
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
);
$order_response = $dpd->setOrder($action, $settings, $ship_address, $parcel_data, $parcel_shop_id);

// Make multiple orders request
// Fill with your values/data
$settings = array(
  'ShipDate' => '',
  'LabelSize' => '',
  'LabelStartPosition' => '',
);
$orders_data = array(
  // First order
  array(
   'ship_address' => array(
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
   'parcel_data' => array(
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
);
$orders_response = $dpd->setOrders('StartOrder', $settings, $orders_data);

// Make Parcel request by its number
$parcel_response = $dpd->getParcelLifeCycle('0xxxxxxxxxxxxx');

// Make an search shop (ShopFinder) request
// Fill with your values/data
$search_address = array(
  'Street' => '',
  'HouseNo' => '',
  'ZipCode' => '',
  'City' => '',
  'Country' => '',
);
$shopfinder_response = $dpd->getParcelShopFinderByAddress($search_address, 10, 0, 'null');

// Make a search shop (Shop Finder) request
// Fill with geo data
$shopfinder_reponse = $dpd->getParcelShopFinderByGeoData($longitude, $latitude, $hide_on_closed_at, $need_service, $max_results = 100);
// Make Zip Code Rules request (get shipping information for a pick-up point).
$zip_code_rules_object = $dpd->getZipCodeRules();
