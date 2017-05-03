import React, { Component } from 'react'
import {
    View,
    Text,
    TouchableOpacity,
    Image,
    Button,
    PixelRatio,
    StyleSheet,
    NativeModules,
} from 'react-native'
import ImagePicker from 'react-native-image-picker'
import Video from 'react-native-video'

class App extends Component {
    constructor(props) {
        super(props)
        this.state = {
            uri: '',
            media: false,
            isProcessing: false,
        }
        this.selectFile = this.selectFile.bind(this)
        this.dehazeFile = this.dehazeFile.bind(this)
        this.resetFile = this.resetFile.bind(this)
    }

    selectFile() {
        const options = {
            quality: 1.0,
            videoQuality: 'high',
            maxWidth: 500,
            maxHeight: 500,
            mediaType: 'mixed',
        }
        ImagePicker.showImagePicker(options, ({ uri, error, didCancel, customButton }) => {
            if (!error && !didCancel && !customButton) {
                const suffix = uri.slice(uri.lastIndexOf('.') + 1).toLowerCase()
                const isImage = ['png', 'jpg', 'jpeg', 'bmp'].includes(suffix)
                const isVideo = ['mp4', 'mov'].includes(suffix)
                this.setState({ uri: uri.slice(7), media: isImage && 'image' || isVideo && 'video' })
            }
        })
    }

    async dehazeFile() {
        const { uri, media, isProcessing } = this.state
        if (!isProcessing) {
            this.setState(state => ({ ...state, isProcessing: true }))
            const dehazedUri = await NativeModules.Dehaze.run(uri, media)
            this.setState({ uri: dehazedUri, isProcessing: false })
        }
    }

    resetFile() {
        this.setState({ uri: '', media: false })
    }

    renderDisplay() {
        const { uri, media } = this.state
        switch (true) {
            case media === 'image': return (
                <Image
                    source={{ isStatic: true, uri }}
                    resizeMode="contain"
                    style={styles.displayContent}
                />
            )
            case media === 'video': return (
                <Video
                    source={{ uri }}
                    resizeMode="contain"
                    muted={false}
                    style={styles.displayContent}
                />
            )
            default: return <Text style={styles.hint}>{uri ? 'Invalid file format' : 'Select a photo or video'}</Text>
        }
    }

    render() {
        const { uri, media, isProcessing } = this.state
        return (
            <View style={styles.container}>
                <TouchableOpacity onPress={this.selectFile}>
                    {this.renderDisplay()}
                </TouchableOpacity>
                {media && (
                    <Button
                        title={isProcessing ? 'Processing...' : 'Dehaze'}
                        onPress={this.dehazeFile}
                        color="#71CA58"
                    />
                )}
                {media && (
                    <Button
                        title="Reset"
                        onPress={this.resetFile}
                        color="#71CA58"
                    />
                )}
            </View>
        )
    }
}

const styles = StyleSheet.create({
    container: {
        flex: 1,
        justifyContent: 'center',
        alignItems: 'center',
        backgroundColor: '#F5FCFF',
    },
    displayContent: {
        flex: 1,
        width: 360,
        maxHeight: 480,
        backgroundColor: 'transparent',
    },
    hint: {
        height: 48,
        fontSize: 24,
        color: "#51BAF2",
    },
});

export default App
