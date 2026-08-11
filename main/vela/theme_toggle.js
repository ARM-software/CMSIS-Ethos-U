(function() {
  const settingName = 'ethosu-color-mode';
  const savedMode = Cookie.readSetting(settingName, 'light');

  DarkModeToggle.userPreference = savedMode === 'dark';
  Cookie.writeSetting(settingName, DarkModeToggle.darkModeEnabled ? 'dark' : 'light');

  const observer = new MutationObserver(function() {
    Cookie.writeSetting(settingName, DarkModeToggle.darkModeEnabled ? 'dark' : 'light');
  });

  observer.observe(document.documentElement, {
    attributes: true,
    attributeFilter: ['class']
  });
})();
