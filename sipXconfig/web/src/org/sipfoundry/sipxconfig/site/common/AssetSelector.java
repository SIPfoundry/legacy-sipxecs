/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.UnsupportedAudioFileException;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

/**
 * Component that allows user to select from existing set of assets (prompts etc.) or upload a new
 * asset.
 */
public abstract class AssetSelector extends BaseComponent {
    private static final Log LOG = LogFactory.getLog(AssetSelector.class);

    @InjectObject("spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Parameter(required = true)
    public abstract String getAsset();

    /**
     * All files in this directory become selectable assets.
     */
    @Parameter(required = true)
    public abstract String getAssetDir();

    @Parameter
    public abstract String getErrorMsg();

    @Parameter(required = true)
    public abstract String getContentType();

    @Parameter(defaultValue = "ognl:true")
    public abstract boolean isEnabled();

    @Parameter(defaultValue = "ognl:true")
    public abstract boolean getCheckForEmptyAssets();

    public abstract void setAsset(String asset);

    public abstract IUploadFile getUploadAsset();

    public abstract void setUploadAsset(IUploadFile uploadFile);

    public abstract String getDeleteAsset();

    public abstract void setDeleteAsset(String asset);

    private static boolean isUploadFileSpecified(IUploadFile file) {
        boolean isSpecified = file != null && !StringUtils.isBlank(file.getFilePath());
        return isSpecified;
    }

    public boolean getAssetExists() {
        return StringUtils.isNotBlank(getAsset());
    }

    public String getDownloadLabel() {
        String key = "download.general";
        if (getContentType().startsWith("audio/")) {
            key = "download.audio";
        }
        String localized = getMessages().getMessage(key);
        return localized;
    }

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this)) {
            AbstractPage page = (AbstractPage) getPage();
            IValidationDelegate validator = TapestryUtils.getValidator(page);
            validateNotEmpty(validator, getErrorMsg());
            validateFileType(validator);
            if (!TapestryUtils.isValid(page)) {
                return;
            }
            checkFileUpload();
            checkDeleteAsset();
        }
    }

    private void checkDeleteAsset() {
        if (getDeleteAsset() != null) {
            File assetFile = new File(getAssetDir(), getDeleteAsset());
            assetFile.delete();
            setAsset(null);
            setDeleteAsset(null);
        }
    }

    private void checkFileUpload() {
        IUploadFile upload = getUploadAsset();
        if (!isUploadFileSpecified(upload)) {
            return;
        }

        FileOutputStream promptWriter = null;
        String fileName = getSystemIndependentFileName(upload.getFilePath());
        try {
            File promptsDir = new File(getAssetDir());
            promptsDir.mkdirs();
            File promptFile = new File(promptsDir, fileName);
            promptWriter = new FileOutputStream(promptFile);
            IOUtils.copy(upload.getStream(), promptWriter);
            setAsset(promptFile.getName());
            setUploadAsset(null);
        } catch (IOException ioe) {
            throw new RuntimeException("Could not upload file " + fileName, ioe);
        } finally {
            IOUtils.closeQuietly(promptWriter);
        }
    }

    /**
     * Extract file name from the path in a system independed way.
     *
     * C:\a\b\c.txt -> c.txt a/b/c.txt => c.txt
     *
     * We cannot use File.getName() here since it only works for filenames from the same operating
     * system. We have to handle the case when Windows file is downloaded on Linux server and vice
     * versa
     *
     * @param filePath full name of the downloaded file in a client sytem format
     * @return base name and extension of the file
     */
    public static String getSystemIndependentFileName(String filePath) {
        if (StringUtils.isEmpty(filePath)) {
            return StringUtils.EMPTY;
        }
        String[] parts = StringUtils.split(filePath, ":/\\");
        return parts[parts.length - 1];
    }

    /**
     * Only call during validation phase
     *
     * @param validator
     * @param errorMsg - if empty we will not validate, if not empty we will record this message
     *        as an error in the validator
     */
    private void validateNotEmpty(IValidationDelegate validator, String errorMsg) {
        if (StringUtils.isEmpty(errorMsg)) {
            return;
        }
        if (StringUtils.isBlank(getAsset()) && getCheckForEmptyAssets() && !isUploadFileSpecified(getUploadAsset())) {
            validator.record(errorMsg, ValidationConstraint.REQUIRED);
        }
    }

    /**
     * Check if uploaded audio files can be played by media server.
     *
     * @throws ValidatorException if audio file is uploaded but cannot be validated
     */
    private void validateFileType(IValidationDelegate validator) {
        if (!getContentType().equals("audio/x-wav")) {
            // only validate audio files
            return;
        }
        IUploadFile upload = getUploadAsset();
        if (!isUploadFileSpecified(upload)) {
            return;
        }
        if (isAcceptedAudioFormat(upload.getStream())) {
            return;
        }

        String error = getMessages().format("error.badWavFormat", upload.getFileName());
        validator.record(new ValidatorException(error));
    }

    /**
     * It is required that the label field is attached to field component
     *
     * @return prompt component
     */
    public IFormComponent getPrompt() {
        return (IFormComponent) getComponent("prompt");
    }

    public boolean isDisabled() {
        return !isEnabled();
    }

    public static boolean isAcceptedAudioFormat(InputStream stream) {
        try {
            InputStream testedStream = stream;
            // HACK: looks like in openjdk InputStream does not support mark/reset
            if (!stream.markSupported()) {
                // getAudioInputStream depends on mark reset do we wrap buffered input stream
                // around passed stream
                testedStream = new BufferedInputStream(stream);
            }
            AudioInputStream audio = AudioSystem.getAudioInputStream(new BufferedInputStream(testedStream));
            AudioFormat format = new AudioFormat(AudioFormat.Encoding.PCM_SIGNED, 8000, // sample
                    // rate
                    16, // bits per sample
                    1, // channels
                    2, // frame rate
                    8000, // frame size
                    false); // isBigEndian)
            return format.matches(audio.getFormat());
        } catch (IOException e) {
            LOG.warn("Uploaded file problems.", e);
        } catch (UnsupportedAudioFileException e) {
            LOG.info("Unsupported format", e);
        }
        return false;
    }
}
