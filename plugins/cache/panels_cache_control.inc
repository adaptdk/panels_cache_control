<?php
/**
 * @file
 * Panels cache control plugin.
 */
// Plugin definition
$plugin = array(
  'title' => t("Cache control"),
  'description' => t('Control the page cache by setting the cache control header.'),
  'cache get' => 'panels_cache_control_cache_get_cache',
  'cache set' => 'panels_cache_control_cache_set_cache',
  'cache clear' => 'panels_cache_control_cache_clear_cache',
  'settings form' => 'panels_cache_control_cache_settings_form',
  'settings form submit' => 'panels_cache_control_cache_settings_form_submit',
  'defaults' => array(
    'lifetime' => 0,
    'directive' => 'public',
    'cookie_name' => '',
    'cookie' => '',
    'hash' => '',
    'add_cookie' => FALSE,
    'reference_name' => '',
  ),
);

/**
 * Get cached content.
 */
function panels_cache_control_cache_get_cache($conf, $display, $args, $contexts, $pane = NULL) {
  global $user;
  // Check for certain roles and/or uids, we don't want cache for all users
  $uids = variable_get('cache_control_no_cache_uids', array());
  $roles = variable_get('cache_control_no_cache_roles', array());
  $disable_all_caching = variable_get('cache_control_no_cache', FALSE);
  $lifetime_override = variable_get('cache_control_lifetime_override', FALSE);

  if ($disable_all_caching || in_array($user->uid, $uids) || count(array_intersect($roles, $user->roles)) > 0) {
    return FALSE;
  }

  $header = $conf['directive'];
  if (!empty($conf['lifetime'])) {
    if ($lifetime_override) {
      $header .= ", max-age={$lifetime_override}";
    }
    else {
      $header .= ", max-age={$conf['lifetime']}";
    }
  }
  drupal_add_http_header('Cache-Control', $header);

  // Allow cookie content to be altered
  drupal_alter('panels_cache_control_cookie_content', $conf);

  if (!empty($conf['add_cookie']) && !empty($conf['cookie'])) {
    $cookie = md5(ctools_context_keyword_substitute($conf['cookie'], array(), $contexts));
    setcookie($conf['cookie_name'], 'pcc' . $cookie, 0, '/', $_SERVER['SERVER_NAME']);
    setcookie($conf['reference_name'], md5(session_id() . $cookie . 'secret'), 0, '/', $_SERVER['SERVER_NAME']);
  }

  return FALSE;
}

/**
 * Set cached content.
 */
function panels_cache_control_cache_set_cache($conf, $content, $display, $args, $contexts, $pane = NULL) {
}

/**
 * Dummy functin for clearing cache.
 */
function panels_cache_control_cache_clear_cache($display) {
}

/**
 * Settings form.
 */
function panels_cache_control_cache_settings_form($conf, $display, $pid) {
  $options = drupal_map_assoc(array(15, 30, 60, 120, 180, 240, 300, 600, 900, 1200, 1800, 3600, 7200, 14400, 28800, 43200, 86400, 172800, 259200, 345600, 604800), 'format_interval');
  $form['lifetime'] = array(
    '#title' => t('Lifetime'),
    '#required' => TRUE,
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => $conf['lifetime'],
  );
  $form['directive'] = array(
    '#title' => t('Directive'),
    '#type' => 'select',
    '#options' => array('private' => t('Private'), 'public' => t('Public')),
    '#default_value' => $conf['directive'],
  );
  $form['add_cookie'] = array(
    '#title' => t('Add a cookie'),
    '#type' => 'checkbox',
    '#description' => t('Add a cookie describing user information that can be used.'),
    '#default_value' => $conf['add_cookie'],
  );
  $form['cookie_name'] = array(
    '#title' => t('Cookie name'),
    '#type' => 'textfield',
    '#default_value' => $conf['cookie_name'],
    '#dependency' => array('edit-settings-add-cookie' => array(1)),
    '#description' => t('Put the contents of the cookie here. Replacement patterns are allowed'),
  );
  $form['reference_name'] = array(
    '#title' => t('Reference key name'),
    '#type' => 'textfield',
    '#default_value' => $conf['reference_name'],
    '#dependency' => array('edit-settings-add-cookie' => array(1)),
    '#description' => t('The name of the cookie that will be used as a storage for the reference hash.'),
  );
  $form['cookie'] = array(
    '#title' => t('Cookie content'),
    '#type' => 'textfield',
    '#default_value' => $conf['cookie'],
    '#dependency' => array('edit-settings-add-cookie' => array(1)),
    '#description' => t('Put the contents of the cookie here. Replacement patterns are allowed'),
  );
  $form['contexts'] = array(
    '#title' => t('Substitutions'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  foreach ($display->context as $context) {
    foreach (ctools_context_get_converters('%' . check_plain($context->keyword) . ':', $context) as $keyword => $title) {
      $rows[] = array(
        check_plain($keyword),
        t('@identifier: @title', array('@title' => $title, '@identifier' => $context->identifier)),
      );
    }
  }
  $header = array(t('Keyword'), t('Value'));
  $form['contexts']['context'] = array('#markup' => theme('table', array('header' => $header, 'rows' => $rows)));
  return $form;
}

/**
 *
 */
function panels_cache_control_cache_settings_form_submit($form, &$form_state) {

}

