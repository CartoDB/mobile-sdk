namespace Carto.Ui {
    using Carto.Components;

    internal delegate void UpdateLicenseDelegate(string license);

    internal class MapLicenseManagerListener : LicenseManagerListener {
        private UpdateLicenseDelegate _updateLicense;

        public MapLicenseManagerListener(UpdateLicenseDelegate updateLicense) {
            _updateLicense = updateLicense;
        }

        public override void OnLicenseUpdated(string licenseKey) {
            _updateLicense (licenseKey);
        }
    }
}
