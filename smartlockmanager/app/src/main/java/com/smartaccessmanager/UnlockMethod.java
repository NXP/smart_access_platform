package com.smartaccessmanager;

public class UnlockMethod
{
    public String title;
    public String description;
    public int icon;
    public Class intentClass;
    public int tag;

    public UnlockMethod(String title, String description, int icon, Class intentClass, int tag)
    {
        this.title = title;
        this.description = description;
        this.icon = icon;
        this.intentClass = intentClass;
        this.tag = tag;
    }
}