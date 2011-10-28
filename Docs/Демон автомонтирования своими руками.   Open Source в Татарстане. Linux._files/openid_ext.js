/* $Id$ */

/**
 * Behaviors for OpenID Extensions themes.
 */
Drupal.behaviors.openid_ext_usability = function() {
  // if current theme is Dropdown it should have some settings
  if (Drupal.settings.openid_ext.dropdown) {
    $('#edit-provider').change(function() {
      val = $(this).val()

      if (Drupal.settings.openid_ext.dropdown[val] && Drupal.settings.openid_ext.dropdown[val].title) {
        $('label', $('#edit-input-wrapper')).text(Drupal.settings.openid_ext.dropdown[val].title)
        // remove all classes and leave only default form item class
        $('#edit-input').attr('class', '').addClass('form-text')
        if (Drupal.settings.openid_ext.dropdown[val].icon) {
          $('#edit-input').addClass('with-icon').addClass($('#edit-provider').val())
        }
        $('#edit-input-wrapper').show()
      }
      else {
        $('#edit-input-wrapper').hide()
      }
    }).change()
  }

  // if current theme is Accordion it should have some settings
  if (Drupal.settings.openid_ext.accordion) {
    var accordion_element = $('#' + Drupal.settings.openid_ext.accordion.container_id)
    $('li', accordion_element).each(function() {
      var list_element = $(this)
      var header_element = $('.accordion-header', list_element)
      var content_element = header_element.next()

      list_element.css('list-style', Drupal.settings.openid_ext.accordion.list_style)
      list_element.css('padding', Drupal.settings.openid_ext.accordion.padding)
      list_element.css('margin', Drupal.settings.openid_ext.accordion.margin)
      header_element.addClass('accordion-header')
      content_element.addClass('accordion-content').empty()
      $('a', header_element).click(function() {
        if (!content_element.hasClass('accordion-content-active')) {
          $('.accordion-content-active', accordion_element).each(function() {
            el = $(this)
            el.removeClass('accordion-content-active').slideUp('normal')
            el.parents('li:first').css('list-style', Drupal.settings.openid_ext.accordion.list_style)
          })
          list_element.css('list-style', Drupal.settings.openid_ext.accordion.list_style_active)

          new_provider = $(this).attr('class')

          if (Drupal.settings.openid_ext.accordion[new_provider] && Drupal.settings.openid_ext.accordion[new_provider].title) {
            if (!$('#edit-input-wrapper', content_element).size()) {
              input_element = $('#edit-input-wrapper')
              content_element.append(input_element)
              $('label', input_element).text(Drupal.settings.openid_ext.accordion[new_provider].title)
              input_element.show()
            }
          }
          op_element = $('#edit-op')
          content_element.append(op_element)
          op_element.show()
          $('#edit-provider').val(new_provider)

          content_element.addClass('accordion-content-active')
          content_element.slideDown('normal')
        }
        return false
      })
    })
    $('a', $('li.first', accordion_element)).click()
  }
}