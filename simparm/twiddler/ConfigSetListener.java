interface ConfigSetListener {
    public void notifyOfNewConfigEntry(ConfigEntry e);
    public void notifyOfConfigEntryDeletion(ConfigEntry e);
}
